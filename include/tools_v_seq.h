#pragma once

namespace lua {
namespace tools {

template<int ...>
struct seq {};

template<int N, int ...S>
struct gen_seq : gen_seq<N - 1, N - 1, S...> {};

template<int ...S>
struct gen_seq<0, S...> {
    typedef seq<S...> type;
};
}
}