#pragma once

namespace hs::utils {

template<typename ...T>
struct Overriden : public T... {
    Overriden(T... args):
        T(args)... {};
};

}

