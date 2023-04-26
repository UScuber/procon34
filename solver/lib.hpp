#pragma once

#include <vector>
#include <iostream>
#include <cassert>

using uchar = unsigned char;
using uint = unsigned int;
using ull = unsigned long long;
using ll = long long;

constexpr int max_height = 25;
constexpr int max_width = 25;
constexpr int max_agent_num = 6;

constexpr int dy[] = { -1,0,1,0, -1,1,1,-1 };
constexpr int dx[] = { 0,-1,0,1, -1,-1,1,1 };

int height = 0, width = 0; // fieldの大きさ
int castles_coef = 0, area_coef = 0, wall_coef = 0; // 係数

struct State {
  static const State None;
  static const State Pond;
  static const State WallEnemy;
  static const State WallAlly;
  static const State AreaEnemy;
  static const State AreaAlly;
  static const State Enemy; // 敵のagent
  static const State Ally; // 味方のagent
  static const State Castle;
  static const State Human; // Ally | Enemey
  static const State Wall; // WallAlly | WallEnemy

  inline constexpr State(const uchar v=0) : val(v){}
  inline constexpr State &operator|=(const State s) noexcept{ val |= s.val; return *this; }
  inline constexpr State &operator&=(const State s) noexcept{ val &= s.val; return *this; }
  inline constexpr State &operator^=(const State s) noexcept{ val ^= s.val; return *this; }
  inline constexpr State operator|(const State s) const noexcept{ return State(*this) |= s; }
  inline constexpr State operator&(const State s) const noexcept{ return State(*this) &= s; }
  inline constexpr State operator^(const State s) const noexcept{ return State(*this) ^= s; }
  inline constexpr State operator~() const noexcept{ return State(~val); }
  inline constexpr operator bool() const noexcept{ return val; }
  inline constexpr bool operator==(const State s) const noexcept{ return val == s.val; }
  friend std::ostream &operator<<(std::ostream &os, const State s){ return os << (int)s.val; }

protected:
  uchar val;
};
constexpr State State::None = 0;
constexpr State State::Pond = 1 << 0;
constexpr State State::WallEnemy = 1 << 1;
constexpr State State::WallAlly = 1 << 2;
constexpr State State::AreaEnemy = 1 << 3;
constexpr State State::AreaAlly = 1 << 4;
constexpr State State::Enemy = 1 << 5;
constexpr State State::Ally = 1 << 6;
constexpr State State::Castle = 1 << 7;
constexpr State State::Human = State::Ally | State::Enemy;
constexpr State State::Wall = State::WallAlly | State::WallEnemy;


struct Point {
  int y,x;
  constexpr Point(const int _y=-1, const int _x=-1) : y(_y), x(_x){}
  inline constexpr Point &operator+=(const Point &p){
    y += p.y; x += p.x; return *this;
  }
  inline constexpr Point operator+(const Point &p) const{ return Point(*this) += p; }
  inline constexpr bool operator<(const Point &p) const{
    if(y != p.y) return y < p.y;
    return x < p.x;
  }
  inline constexpr bool operator==(const Point &p) const{ return y == p.y && x == p.x; }
  friend std::istream &operator>>(std::istream &is, Point &p){
    return is >> p.y >> p.x;
  }
  friend std::ostream &operator<<(std::ostream &os, const Point &p){
    return os << p.y << " " << p.x;
  }
};

inline constexpr bool is_valid(const int y, const int x) noexcept{
  return 0 <= y && y < height && 0 <= x && x < width;
}

inline constexpr bool is_valid(const Point &p) noexcept{
  return 0 <= p.y && p.y < height && 0 <= p.x && p.x < width;
}

inline constexpr bool is_neighbor(const Point &p, const Point &q) noexcept{
  return abs(p.y - q.y) + abs(p.x - q.x) <= 1;
}

constexpr Point dmove[] = {
  { -1,0 },
  { 0,-1 },
  { 1,0 },
  { 0,1 },
  { -1,-1 },
  { 1,-1 },
  { 1,1 },
  { -1,1 }
};


inline uint randxor32() noexcept{
  //static uint y = (uint)rand() | (uint)rand() << 16;
  static uint y = 1210253353;
  //std::cerr << y << " ";
  y = y ^ (y << 13); y = y ^ (y >> 17);
  return y = y ^ (y << 5);
}
// returns random [l, r)
inline int rnd(const int l, const int r) noexcept{
  return randxor32() % (r - l) + l;
}
// returns random [0, len)
inline int rnd(const int len) noexcept{
  return randxor32() % len;
}
