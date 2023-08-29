// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2023 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

// ****************************************************************************
// This snippet demonstrates how to setup softbodies.
// ****************************************************************************

#include <ctype.h>
#include "PxPhysicsAPI.h"
#include "../snippetcommon/SnippetPrint.h"
#include "../snippetcommon/SnippetPVD.h"
#include "../snippetutils/SnippetUtils.h"
#include "../snippetkinematicsoftbody/SnippetKinematicSoftBody.h"
#include "../snippetkinematicsoftbody/MeshGenerator.h"
#include "extensions/PxTetMakerExt.h"
#include "extensions/PxSoftBodyExt.h"

using namespace physx;
using namespace meshgenerator;

static PxDefaultAllocator		gAllocator;
static PxDefaultErrorCallback	gErrorCallback;
static PxFoundation*			gFoundation		= NULL;
static PxPhysics*				gPhysics		= NULL;
static PxCudaContextManager*	gCudaContextManager = NULL;
static PxDefaultCpuDispatcher*	gDispatcher		= NULL;
static PxScene*					gScene			= NULL;
static PxMaterial*				gMaterial		= NULL;
static PxPvd*					gPvd			= NULL;
std::vector<SoftBody>			gSoftBodies;

void addSoftBody(PxSoftBody* softBody, const PxFEMParameters& femParams, const PxFEMMaterial& /*femMaterial*/,
	const PxTransform& transform, const PxReal density, const PxReal scale, const PxU32 iterCount/*, PxMaterial* tetMeshMaterial*/)
{
	PxVec4* simPositionInvMassPinned;
	PxVec4* simVelocityPinned;
	PxVec4* collPositionInvMassPinned;
	PxVec4* restPositionPinned;

	PxSoftBodyExt::allocateAndInitializeHostMirror(*softBody, gCudaContextManager, simPositionInvMassPinned, simVelocityPinned, collPositionInvMassPinned, restPositionPinned);
	
	const PxReal maxInvMassRatio = 50.f;

	softBody->setParameter(femParams);
	//softBody->setMaterial(femMaterial);
	softBody->setSolverIterationCounts(iterCount);

	PxSoftBodyExt::transform(*softBody, transform, scale, simPositionInvMassPinned, simVelocityPinned, collPositionInvMassPinned, restPositionPinned);
	PxSoftBodyExt::updateMass(*softBody, density, maxInvMassRatio, simPositionInvMassPinned);
	PxSoftBodyExt::copyToDevice(*softBody, PxSoftBodyDataFlag::eALL, simPositionInvMassPinned, simVelocityPinned, collPositionInvMassPinned, restPositionPinned);

	SoftBody sBody(softBody, gCudaContextManager);

	gSoftBodies.push_back(sBody);

	PX_PINNED_HOST_FREE(gCudaContextManager, simPositionInvMassPinned);
	PX_PINNED_HOST_FREE(gCudaContextManager, simVelocityPinned);
	PX_PINNED_HOST_FREE(gCudaContextManager, collPositionInvMassPinned);
	PX_PINNED_HOST_FREE(gCudaContextManager, restPositionPinned);
}

static PxSoftBody* createSoftBody(const PxCookingParams& params, const PxArray<PxVec3>& triVerts, const PxArray<PxU32>& triIndices, bool useCollisionMeshForSimulation = false)
{
	PxFEMSoftBodyMaterial* material = PxGetPhysics().createFEMSoftBodyMaterial(1e+6f, 0.45f, 0.5f);
	material->setDamping(0.005f);

	PxSoftBodyMesh* softBodyMesh;

	PxU32 numVoxelsAlongLongestAABBAxis = 8;

	PxSimpleTriangleMesh surfaceMesh;
	surfaceMesh.points.count = triVerts.size();
	surfaceMesh.points.data = triVerts.begin();
	surfaceMesh.triangles.count = triIndices.size() / 3;
	surfaceMesh.triangles.data = triIndices.begin();

	if (useCollisionMeshForSimulation)
	{
		softBodyMesh = PxSoftBodyExt::createSoftBodyMeshNoVoxels(params, surfaceMesh, gPhysics->getPhysicsInsertionCallback());
	}
	else 
	{
		softBodyMesh = PxSoftBodyExt::createSoftBodyMesh(params, surfaceMesh, numVoxelsAlongLongestAABBAxis, gPhysics->getPhysicsInsertionCallback());
	}	

	//Alternatively one can cook a softbody mesh in a single step
	//tetMesh = cooking.createSoftBodyMesh(simulationMeshDesc, collisionMeshDesc, softbodyDesc, physics.getPhysicsInsertionCallback());
	PX_ASSERT(softBodyMesh);

	if (!gCudaContextManager)
		return NULL;
	PxSoftBody* softBody = gPhysics->createSoftBody(*gCudaContextManager);
	if (softBody)
	{
		PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE;

		PxFEMSoftBodyMaterial* materialPtr = PxGetPhysics().createFEMSoftBodyMaterial(1e+6f, 0.45f, 0.5f);
		materialPtr->setMaterialModel(PxFEMSoftBodyMaterialModel::eNEO_HOOKEAN);
		PxTetrahedronMeshGeometry geometry(softBodyMesh->getCollisionMesh());
		PxShape* shape = gPhysics->createShape(geometry, &materialPtr, 1, true, shapeFlags);
		if (shape)
		{
			softBody->attachShape(*shape);
			shape->setSimulationFilterData(PxFilterData(0, 0, 2, 0));
		}
		softBody->attachSimulationMesh(*softBodyMesh->getSimulationMesh(), *softBodyMesh->getSoftBodyAuxData());

		gScene->addActor(*softBody);

		PxFEMParameters femParams;
		addSoftBody(softBody, femParams, *material, PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxIdentity)), 100.f, 1.0f, 30);
		softBody->setSoftBodyFlag(PxSoftBodyFlag::eDISABLE_SELF_COLLISION, true);
	}
	return softBody;
}

static void createSoftbodies(const PxCookingParams& params)
{
	PxArray<PxVec3> triVerts;
	PxArray<PxU32> triIndices;
	
	PxReal maxEdgeLength = 0.75f;

	createCube(triVerts, triIndices, PxVec3(0, 0, 0), PxVec3(2.5f, 10, 2.5f));
	PxRemeshingExt::limitMaxEdgeLength(triIndices, triVerts, maxEdgeLength);

	PxVec3 position(0, 5.0f, 0);
	for (PxU32 i = 0; i < triVerts.size(); ++i)
	{
		PxVec3& p = triVerts[i];
		PxReal corr = PxSqrt(p.x*p.x + p.z*p.z);
		if (corr != 0)
			corr = PxMax(PxAbs(p.x), PxAbs(p.z)) / corr;
		PxReal scaling = 0.75f + 0.5f * (PxCos(1.5f*p.y) + 1.0f);
		p.x *= scaling * corr;
		p.z *= scaling * corr;
		p += position;
	}
	PxRemeshingExt::limitMaxEdgeLength(triIndices, triVerts, maxEdgeLength);


	PxSoftBody* softBody = createSoftBody(params, triVerts, triIndices, true);
	

	SoftBody* sb = &gSoftBodies[0];
	sb->copyDeformedVerticesFromGPU();

	PxCudaContextManager* cudaContextManager = gScene->getCudaContextManager();
	PxU32 vertexCount = sb->mSoftBody->getSimulationMesh()->getNbVertices();


	PxVec4* kinematicTargets = PX_PINNED_HOST_ALLOC_T(PxVec4, cudaContextManager, vertexCount);	
	PxVec4* positionInvMass = sb->mPositionsInvMass;
	for (PxU32 i = 0; i < vertexCount; ++i)
	{
		PxVec4& p = positionInvMass[i];	
		bool kinematic = false;
		if (i < triVerts.size())
		{
			if (p.y > 9.9f)
				kinematic = true;

			if (p.y > 5 - 0.1f && p.y < 5 + 0.1f)
				kinematic = true;

			if (p.y < 0.1f)
				kinematic = true;
		}
		
		kinematicTargets[i] = PxConfigureSoftBodyKinematicTarget(p, kinematic);
	}

	PxVec4* kinematicTargetsD = PX_DEVICE_ALLOC_T(PxVec4, cudaContextManager, vertexCount);
	cudaContextManager->getCudaContext()->memcpyHtoD(reinterpret_cast<CUdeviceptr>(softBody->getSimPositionInvMassBufferD()), positionInvMass, vertexCount * sizeof(PxVec4));
	cudaContextManager->getCudaContext()->memcpyHtoD(reinterpret_cast<CUdeviceptr>(kinematicTargetsD), kinematicTargets, vertexCount * sizeof(PxVec4));
	softBody->setKinematicTargetBufferD(kinematicTargetsD, PxSoftBodyFlag::ePARTIALLY_KINEMATIC);
	
	sb->mTargetPositionsH = kinematicTargets;
	sb->mTargetPositionsD = kinematicTargetsD;
	sb->mTargetCount = vertexCount;
}

void initPhysics(bool /*interactive*/)
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport,PxPvdInstrumentationFlag::eALL);
	
	// initialize cuda
	PxCudaContextManagerDesc cudaContextManagerDesc;
	gCudaContextManager = PxCreateCudaContextManager(*gFoundation, cudaContextManagerDesc, PxGetProfilerCallback());
	if (gCudaContextManager && !gCudaContextManager->contextIsValid())
	{
		gCudaContextManager->release();
		gCudaContextManager = NULL;
		printf("Failed to initialize cuda context.\n");
	}

	PxTolerancesScale scale;
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, scale, true, gPvd);
	PxInitExtensions(*gPhysics, gPvd);

	PxCookingParams params(scale);
	params.meshWeldTolerance = 0.001f;
	params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES);
	params.buildTriangleAdjacencies = false;
	params.buildGPUData = true;

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

	if (!sceneDesc.cudaContextManager)
		sceneDesc.cudaContextManager = gCudaContextManager;
	
	sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
	sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;

	PxU32 numCores = SnippetUtils::getNbPhysicalCores();
	gDispatcher = PxDefaultCpuDispatcherCreate(numCores == 0 ? 0 : numCores - 1);
	sceneDesc.cpuDispatcher	= gDispatcher;
	sceneDesc.filterShader	= PxDefaultSimulationFilterShader;
	sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;

	sceneDesc.sceneQueryUpdateMode = PxSceneQueryUpdateMode::eBUILD_ENABLED_COMMIT_DISABLED;
	sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
	sceneDesc.gpuMaxNumPartitions = 8;

	sceneDesc.solverType = PxSolverType::eTGS;

	gScene = gPhysics->createScene(sceneDesc);
	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if(pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.f);

	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0,1,0,0), *gMaterial);
	gScene->addActor(*groundPlane);

	createSoftbodies(params);

	// Setup rigid bodies
	const PxReal dynamicsDensity = 10;
	const PxReal boxSize = 0.5f;
	const PxReal spacing = 0.6f;
	const PxReal boxMass = boxSize * boxSize * boxSize * dynamicsDensity;
	const PxU32 gridSizeA = 13;
	const PxU32 gridSizeB = 3;
	const PxReal initialRadius = 1.65f;
	const PxReal distanceJointStiffness = 500.0f;
	const PxReal distanceJointDamping = 0.5f;
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(0.5f * boxSize, 0.5f * boxSize, 0.5f * boxSize), *gMaterial);
	shape->setDensityForFluid(dynamicsDensity);
	PxArray<PxRigidDynamic*> rigids;
	for (PxU32 i = 0; i < gridSizeA; ++i)
		for (PxU32 j = 0; j < gridSizeB; ++j)
		{
			PxReal x = PxCos((2 * PxPi*i) / gridSizeA);
			PxReal y = PxSin((2 * PxPi*i) / gridSizeA);
			PxVec3 pos = PxVec3((x*j)*spacing + x * initialRadius, 8, (y *j)*spacing + y * initialRadius);

			PxReal d = 0.0f;
			{
				PxReal x2 = PxCos((2 * PxPi*(i + 1)) / gridSizeA);
				PxReal y2 = PxSin((2 * PxPi*(i + 1)) / gridSizeA);
				PxVec3 pos2 = PxVec3((x2*j)*spacing + x2 * initialRadius, 8, (y2 *j)*spacing + y2 * initialRadius);
				d = (pos - pos2).magnitude();
			}

			PxRigidDynamic* body = gPhysics->createRigidDynamic(PxTransform(pos));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, boxMass);
			gScene->addActor(*body);
			rigids.pushBack(body);

			if (j > 0)
			{
				PxDistanceJoint* joint = PxDistanceJointCreate(*gPhysics, rigids[rigids.size() - 2], PxTransform(PxIdentity), body, PxTransform(PxIdentity));
				joint->setMaxDistance(spacing);
				joint->setMinDistance(spacing*0.5f);
				joint->setDistanceJointFlags(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED | PxDistanceJointFlag::eMIN_DISTANCE_ENABLED | PxDistanceJointFlag::eSPRING_ENABLED);
				joint->setStiffness(distanceJointStiffness);
				joint->setDamping(distanceJointDamping);
				joint->setConstraintFlags(PxConstraintFlag::eCOLLISION_ENABLED);
			}

			if (i > 0)
			{
				PxDistanceJoint* joint = PxDistanceJointCreate(*gPhysics, rigids[rigids.size() - gridSizeB - 1], PxTransform(PxIdentity), body, PxTransform(PxIdentity));
				joint->setMaxDistance(d);
				joint->setMinDistance(d*0.5f);
				joint->setDistanceJointFlags(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED | PxDistanceJointFlag::eMIN_DISTANCE_ENABLED | PxDistanceJointFlag::eSPRING_ENABLED);
				joint->setStiffness(distanceJointStiffness);
				joint->setDamping(distanceJointDamping);
				joint->setConstraintFlags(PxConstraintFlag::eCOLLISION_ENABLED);
				if (i == gridSizeA - 1)
				{
					PxDistanceJoint* joint2 = PxDistanceJointCreate(*gPhysics, rigids[j], PxTransform(PxIdentity), body, PxTransform(PxIdentity));
					joint2->setMaxDistance(d);
					joint2->setMinDistance(d*0.5f);
					joint2->setDistanceJointFlags(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED | PxDistanceJointFlag::eMIN_DISTANCE_ENABLED | PxDistanceJointFlag::eSPRING_ENABLED);
					joint2->setStiffness(distanceJointStiffness);
					joint2->setDamping(distanceJointDamping);
					joint->setConstraintFlags(PxConstraintFlag::eCOLLISION_ENABLED);
				}
			}
		}
	shape->release();
}

PxReal simTime = 0.0f;
void stepPhysics(bool /*interactive*/)
{
	const PxReal dt = 1.0f / 60.f;

	gScene->simulate(dt);
	gScene->fetchResults(true);

	for (PxU32 i = 0; i < gSoftBodies.size(); i++)
	{
		SoftBody* sb = &gSoftBodies[i]; 
		sb->copyDeformedVerticesFromGPU();	


		PxCudaContextManager* cudaContextManager = sb->mCudaContextManager;
		//Update the kinematic targets to get some motion
		if (i == 0)
		{
			PxReal scaling = PxMin(0.01f, simTime * 0.1f);
			PxReal velocity = 1.0f;
			for (PxU32 j = 0; j < sb->mTargetCount; ++j)
			{
				PxVec4& target = sb->mTargetPositionsH[j];
				if (target.w == 0.0f)
				{
					PxReal phase = target.y*2.0f;
					target.x += scaling * PxSin(velocity * simTime + phase);
					target.z += scaling * PxCos(velocity * simTime + phase);
				}
			}

			PxScopedCudaLock _lock(*cudaContextManager);
			cudaContextManager->getCudaContext()->memcpyHtoD(reinterpret_cast<CUdeviceptr>(sb->mTargetPositionsD), sb->mTargetPositionsH, sb->mTargetCount * sizeof(PxVec4));
		}
	}
	simTime += dt;
}
	
void cleanupPhysics(bool /*interactive*/)
{
	for (PxU32 i = 0; i < gSoftBodies.size(); i++)
		gSoftBodies[i].release();
	gSoftBodies.clear();

	PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);
	PX_RELEASE(gPhysics);
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();
	PxCloseExtensions();

	gCudaContextManager->release();
	PX_RELEASE(gFoundation);

	printf("Snippet Kinematic Softbody done.\n");
}

int snippetMain(int, const char*const*)
{
#ifdef RENDER_SNIPPET
	extern void renderLoop();
	renderLoop();
#else
	static const PxU32 frameCount = 100;
	initPhysics(false);
	for(PxU32 i=0; i<frameCount; i++)
		stepPhysics(false);
	cleanupPhysics(false);
#endif

	return 0;
}
