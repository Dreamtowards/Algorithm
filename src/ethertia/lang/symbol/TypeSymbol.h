//
// Created by Dreamtowards on 2022/6/4.
//

#ifndef ETHERTIA_TYPESYMBOL_H
#define ETHERTIA_TYPESYMBOL_H


class TypeSymbol
{
    uint16_t typsize = 0;

    virtual void makv() {}
public:

    // size in bytes.
    uint16_t getTypesize() const {
        return typsize;
    }

    void __typesize(uint16_t n) {
        typsize = n;
    }
};

#endif //ETHERTIA_TYPESYMBOL_H
