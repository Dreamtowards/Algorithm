//
// Created by Dreamtowards on 2022/4/22.
//

#ifndef ETHERTIA_ETHERTIA_H
#define ETHERTIA_ETHERTIA_H

#include <glm/glm.hpp>
#include <stdexcept>

//#include <ethertia/render/Camera.h>
//#include <ethertia/init/HitCursor.h>
//#include <ethertia/util/Timer.h>
//#include <ethertia/util/Profiler.h>
//#include <ethertia/util/Scheduler.h>


#include <cassert>
#ifndef NDEBUG
#define ET_DEBUG
#endif


class World;
class WorldInfo;
class EntityPlayer;
class Window;

namespace Ethertia
{
    /// @worldinfo is only required when creating a new world, at the same time @savedir should also be an empty or non-existing folder.
    void LoadWorld(const std::string& savedir, const WorldInfo* worldinfo = nullptr);

    void UnloadWorld();

    void DispatchCommand(const std::string& cmd);

    void PrintMessage(const std::string& msg);

    bool& IsRunning();
    void Shutdown();

    bool InMainThread();

    // ClientSide Only
    //World* GetWorld();
    //EntityPlayer* GetPlayer();
    //Timer& GetTimer();
    //Scheduler& GetScheduler();
    //Scheduler& GetAsyncScheduler();
    //HitCursor& GetHitCursor();
    //Profiler& GetProfiler();
    //Camera& GetCamera();


    float GetDelta();  /// (previous) frame delta time in seconds.
    float GetPreciseTime();  /// precise program-running-time in seconds. !not epoch timestamp.

    bool& IsIngame();  // is controlling game



    struct Viewport
    {
        float x, y, width, height;
    
        // width/height. return 0 if height==0.
        float getAspectRatio() const {
            if (height==0) return 0;
            return width/height;
        }
        float right() const { return x+width; }
        float bottom() const { return y+height; }
    
    };
    
    // game viewport. useful when game required to be rendered in a specific area.
    const Ethertia::Viewport& GetViewport();
    
    //struct Version
    //{
    //    inline static const int major = 0;
    //    inline static const int minor = 0;
    //    inline static const int revision = 3;
    //    inline static const char* snapshot = "23w10";  // release: nullptr.
    //
    //    static const std::string& version_name() {
    //        static std::string _Inst = snapshot ? Strings::fmt("{} *{}.{}.{}", snapshot, major, minor, revision)
    //                                            : Strings::fmt("{}.{}.{}", major, minor, revision);
    //        return _Inst;
    //    }
    //    static const std::string& name() {
    //        static std::string _Inst = Strings::fmt("Ethertia Alpha {}", version_name());
    //        return _Inst;
    //    }
    //};
}

//class Ethertia
//{
//public:
//    Ethertia() = delete;
//
//    static void loadWorld(const std::string& savedir, const WorldInfo* worldinfo = nullptr);
//    static void unloadWorld();
//
//    // send chat message (@cmd not '/' leading) or dispatch command ('/' leading).
//    static void dispatchCommand(const std::string& cmd);
//
//    static void notifyMessage(const std::string& msg);  // print @msg on Message Box.
//
//
//    static bool& isRunning();
//    static void shutdown() { isRunning() = false; }     // not immediately shutdown, but after this frame.
//
//
//    static bool inMainThread() { return getScheduler().inThread(); }  // is call from main thread.
//
//
//
//    static World* getWorld();
//    static EntityPlayer* getPlayer();
//    static Window& getWindow();
//    static Timer& getTimer();
//    static Scheduler& getScheduler();
//    static Scheduler& getAsyncScheduler();
//    static HitCursor& getHitCursor();
//    static Profiler& getProfiler();
//    static Camera& getCamera();
//
//    static float getDelta();        // (previous) frame delta time in seconds.
//    static float getPreciseTime();  // precise program-running-time in seconds. !not epoch timestamp.
//
//    // is controlling the game. (mouse grabbed, wsad etc.)
//    static bool& isIngame();
//
//
//
//
//    struct Viewport
//    {
//        float x, y, width, height;
//
//        // width/height. return 0 if height==0.
//        float getAspectRatio() const {
//            if (height==0) return 0;
//            return width/height;
//        }
//        float right() const { return x+width; }
//        float bottom() const { return y+height; }
//
//    };
//
//    // game viewport. useful when game required to be rendered in a specific area.
//    static const Ethertia::Viewport& getViewport();
//
//    struct Version
//    {
//        inline static const int major = 0;
//        inline static const int minor = 0;
//        inline static const int revision = 3;
//        inline static const char* snapshot = "23w10";  // release: nullptr.
//
//        static const std::string& version_name() {
//            static std::string _Inst = snapshot ? Strings::fmt("{} *{}.{}.{}", snapshot, major, minor, revision)
//                                                : Strings::fmt("{}.{}.{}", major, minor, revision);
//            return _Inst;
//        }
//        static const std::string& name() {
//            static std::string _Inst = Strings::fmt("Ethertia Alpha {}", version_name());
//            return _Inst;
//        }
//    };
//
//
//};
//


#define PROFILE_VN_CONCAT_INNR(a, b) a ## b
#define PROFILE_VN_CONCAT(a, b) PROFILE_VN_CONCAT_INNR(a, b)
#define PROFILE(p, x) auto PROFILE_VN_CONCAT(_profiler, __COUNTER__) = p.push_ap(x)
#define PROFILE_X(x) auto PROFILE_VN_CONCAT(_profiler, __COUNTER__) = Ethertia::GetProfiler().push_ap(x)


#endif //ETHERTIA_ETHERTIA_H
