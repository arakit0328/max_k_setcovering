#pragma once
#include <random>
#include <cstdint>

class Rand {
private:
    std::mt19937 mt;            //32ビット版メルセンヌ・ツイスタ
    std::random_device rd;      //非決定論的な乱数

public:
    // コンストラクタ(初期化)
    Rand() { mt.seed(rd()); }

    //初期値
    void seed() {
        mt.seed(rd());
    }
    void seed(const std::uint_fast32_t seed_) {
        mt.seed(seed_);
    }

    //通常の乱数
    std::uint_fast32_t operator()() {
        return mt();
    }
    //0～最大値-1 (余りの範囲の一様分布乱数)
    std::int_fast32_t operator()(const std::int_fast32_t max_) {
        std::uniform_int_distribution<> uid(0, ((max_ > 0) ? (std::int_fast32_t)max_ - 1 : 0));
        return uid(mt);
    }
    //最小値～最大値
    std::int_fast32_t operator()(const std::int_fast32_t min_, const std::int_fast32_t max_) {
        std::uniform_int_distribution<> uid((min_ <= max_) ? min_ : max_, (min_ <= max_) ? max_ : min_);
        return uid(mt);
    }
    //確率
    bool randBool(const double probability_) {
        std::bernoulli_distribution uid(probability_);
        return uid(mt);
    }
    bool randBool() {
        std::uniform_int_distribution<> uid(0, 1);
        return ((uid(mt)) ? true : false);
    }
};

//static thread_local Rand rnd;
