// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <fstream>
#include <gmp.h>
#include <iostream>
#include <mp++/mp++.hpp>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include "simple_timer.hpp"

#if defined(MPPP_BENCHMARK_BOOST)
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <gmp.h>
#endif

#if defined(MPPP_BENCHMARK_FLINT)
#include <flint/flint.h>
#include <flint/fmpzxx.h>
#endif

using namespace mppp;
using namespace mppp_bench;

#if defined(MPPP_BENCHMARK_BOOST)
using cpp_int = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>, boost::multiprecision::et_off>;
using mpz_int = boost::multiprecision::number<boost::multiprecision::gmp_int, boost::multiprecision::et_off>;
#endif

#if defined(MPPP_BENCHMARK_FLINT)
using fmpzxx = flint::fmpzxx;
#endif

static std::mt19937 rng;

using integer_t = integer<2>;
static const std::string name = "integer2_vec_mul_unsigned";

constexpr auto size = 30000000ul;

template <typename T>
static inline std::tuple<std::vector<T>, std::vector<T>, std::vector<T>> get_init_vectors(double &init_time)
{
    rng.seed(0);
    std::uniform_int_distribution<unsigned> dist(1u, 7u);
    simple_timer st;
    std::vector<T> v1(size), v2(size), v3(size);
    std::generate(v1.begin(), v1.end(), [&dist]() { return static_cast<T>(T(dist(rng)) << (GMP_NUMB_BITS / 2)); });
    std::generate(v2.begin(), v2.end(), [&dist]() { return static_cast<T>(T(dist(rng)) << (GMP_NUMB_BITS / 2)); });
    std::cout << "\nInit runtime: ";
    init_time = st.elapsed();
    return std::make_tuple(std::move(v1), std::move(v2), std::move(v3));
}

int main()
{
    // Warm up.
    for (auto volatile counter = 0ull; counter < 1000000000ull; ++counter) {
    }
    // Setup of the python output.
    std::string s = "# -*- coding: utf-8 -*-\n"
                    "def get_data():\n"
                    "    import pandas\n"
                    "    data = [";
    {
        std::cout << "\n\nBenchmarking mp++.";
        simple_timer st1;
        double init_time;
        auto p = get_init_vectors<integer_t>(init_time);
        s += "['mp++','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            integer_t ret(0);
            for (auto i = 0ul; i < size; ++i) {
                mul(std::get<2>(p)[i], std::get<0>(p)[i], std::get<1>(p)[i]);
            }
            for (auto i = 0ul; i < size; ++i) {
                add(ret, ret, std::get<2>(p)[i]);
            }
            std::cout << ret << '\n';
            s += "['mp++','arithmetic'," + std::to_string(st2.elapsed()) + "],";
            std::cout << "\nArithmetic runtime: ";
        }
        s += "['mp++','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << "\nTotal runtime: ";
    }
#if defined(MPPP_BENCHMARK_BOOST)
    {
        std::cout << "\n\nBenchmarking cpp_int.";
        simple_timer st1;
        double init_time;
        auto p = get_init_vectors<cpp_int>(init_time);
        s += "['Boost (cpp_int)','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            cpp_int ret(0);
            for (auto i = 0ul; i < size; ++i) {
                std::get<2>(p)[i] = std::get<0>(p)[i] * std::get<1>(p)[i];
            }
            for (auto i = 0ul; i < size; ++i) {
                ret += std::get<2>(p)[i];
            }
            std::cout << ret << '\n';
            s += "['Boost (cpp_int)','arithmetic'," + std::to_string(st2.elapsed()) + "],";
            std::cout << "\nArithmetic runtime: ";
        }
        s += "['Boost (cpp_int)','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << "\nTotal runtime: ";
    }
    {
        std::cout << "\n\nBenchmarking mpz_int.";
        simple_timer st1;
        double init_time;
        auto p = get_init_vectors<mpz_int>(init_time);
        s += "['Boost (mpz_int)','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            mpz_int ret(0);
            for (auto i = 0ul; i < size; ++i) {
                ::mpz_mul(std::get<2>(p)[i].backend().data(), std::get<0>(p)[i].backend().data(),
                          std::get<1>(p)[i].backend().data());
            }
            for (auto i = 0ul; i < size; ++i) {
                ::mpz_add(ret.backend().data(), ret.backend().data(), std::get<2>(p)[i].backend().data());
            }
            std::cout << ret << '\n';
            s += "['Boost (mpz_int)','arithmetic'," + std::to_string(st2.elapsed()) + "],";
            std::cout << "\nArithmetic runtime: ";
        }
        s += "['Boost (mpz_int)','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << "\nTotal runtime: ";
    }
#endif
#if defined(MPPP_BENCHMARK_FLINT)
    {
        std::cout << "\n\nBenchmarking fmpzxx.";
        simple_timer st1;
        double init_time;
        auto p = get_init_vectors<fmpzxx>(init_time);
        s += "['FLINT','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            fmpzxx ret(0);
            for (auto i = 0ul; i < size; ++i) {
                ::fmpz_mul(std::get<2>(p)[i]._data().inner, std::get<0>(p)[i]._data().inner,
                           std::get<1>(p)[i]._data().inner);
            }
            for (auto i = 0ul; i < size; ++i) {
                ::fmpz_add(ret._data().inner, ret._data().inner, std::get<2>(p)[i]._data().inner);
            }
            std::cout << ret << '\n';
            s += "['FLINT','arithmetic'," + std::to_string(st2.elapsed()) + "],";
            std::cout << "\nArithmetic runtime: ";
        }
        s += "['FLINT','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << "\nTotal runtime: ";
    }
#endif
    s += "]\n"
         "    retval = pandas.DataFrame(data)\n"
         "    retval.columns = ['Library','Task','Runtime (ms)']\n"
         "    return retval\n\n"
         "if __name__ == '__main__':\n"
         "    import matplotlib as mpl\n"
         "    mpl.use('Agg')\n"
         "    from matplotlib.pyplot import legend\n"
         "    import seaborn as sns\n"
         "    df = get_data()\n"
         "    g = sns.factorplot(x='Library', y = 'Runtime (ms)', hue='Task', data=df, kind='bar', palette='muted', "
         "legend = False, size = 5.5, aspect = 1.5)\n"
         "    legend(loc='upper right')\n"
         "    g.fig.suptitle('"
         + name + "')\n"
                  "    g.savefig('"
         + name + ".svg', bbox_inches='tight')\n";
    std::ofstream of(name + ".py", std::ios_base::trunc);
    of << s;
}