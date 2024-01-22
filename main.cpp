#include <Siv3D.hpp>

int speed = 1;
int collisionCount = 0;

struct obj {
  enum class obj_type {
    STATIC,
    DYNAMIC,
  };

  obj(double x, double y, double w, double h, double mass) {
    velocity = 0;
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _m = mass;
  }

  obj setVelocity(double _velocity) {
    velocity = _velocity;
    return *this;
  }

  obj setType(obj_type type) {
    _type = type;
    return *this;
  }

  Rect rect(void) {
    return Rect(_x, _y, _w, _h);
  }

  double velocity;
  double _x;
  double _y;
  double _w;
  double _h;
  double _m;
  obj_type _type = obj_type::STATIC;
};

std::set<std::pair<int, int>> getCollisions(const Array<obj> &objs) {
  std::set<std::pair<int, int>> collisions;

  if (objs[2]._x <= objs[0]._x + objs[0]._w)
    collisions.insert({0, 2});
  if (objs[1]._x <= objs[2]._x + objs[2]._w)
    collisions.insert({1, 2});

  return collisions;
}

double calcCollisions(Array<obj> &objs, std::set<std::pair<int, int>> const collisions, double delta) {
  for (auto &&[a, b] : collisions) {
    const double va = objs[a].velocity, vb = objs[b].velocity;

    if (objs[a]._type == obj::obj_type::STATIC)
      objs[b].velocity *= -1;
    else
      objs[a].velocity = (va * (objs[a]._m - objs[b]._m) + 2 * objs[b]._m * vb) / (objs[a]._m + objs[b]._m);
    if (objs[b]._type == obj::obj_type::STATIC)
      objs[a].velocity *= -1;
    else
      objs[b].velocity = va - vb + objs[a].velocity;

    double x;
    double time;
    bool flag = false;
    if (abs(va) < abs(vb))
      flag = 0 < vb;
    else
      flag = va < 0;

    if (flag)
      x = objs[b]._x + (vb < 0 ? 0 : objs[b]._w) - objs[a]._x;
    else
      x = objs[a]._x + (va < 0 ? 0 : objs[a]._w) - objs[b]._x;

    time = x / abs(va - vb);

    objs[a]._x += -time * va + (delta - time) * objs[a].velocity;
    objs[b]._x += -time * vb + (delta - time) * objs[b].velocity;
    delta -= time;
  }

  return delta;
}

void drawAll(Array<obj> &objs, const Font font) {
  for (size_t i = 0; i < objs.size(); i++) {
    auto rect = objs[i].rect();
    if (rect.x + rect.w > 1000) {
      rect = Rect(Arg::center(300 * i, 450), rect.size);
      Rect(rect.x - 40, rect.y - 40, rect.w + 80, rect.h + 80).drawFrame(3);
    }
    rect.draw(HSV(100 + i * 70, 0.5, 1));
    font(U"pos:", (int)objs[i]._x).drawAt(rect.pos + Vec2{rect.w / 2, -27});
    font(U"v:", (int)objs[i].velocity).drawAt(rect.pos + Vec2{rect.w / 2, -10});
  }

  if (SimpleGUI::Button(U"speed up", Vec2{0, 0}))
    if (speed < 10e10)
      speed *= 2;
  if (SimpleGUI::Button(U"speed down", Vec2{0, 35}))
    if (1 < speed)
      speed /= 2;

  font(U"collision count:", collisionCount).drawAt(500, 10);
  font(U"speed:x", speed).drawAt(500, 30);
}

void Main() {
  Window::Resize(1000, 600);

  Array<obj> objs;
  objs << obj(0, 0, 100, 600, 1);
  objs << obj(400, 250, 100, 100, 10000).setVelocity(-100).setType(obj::obj_type::DYNAMIC);
  objs << obj(300, 275, 50, 50, 1).setVelocity(0).setType(obj::obj_type::DYNAMIC);

  const Font font(20);

  while (System::Update()) {
    std::set<std::pair<int, int>> collisions;
    double delta = double(Scene::DeltaTime()) * speed;
    for (int i = 0; i < objs.size(); i++)
      objs[i]._x += objs[i].velocity * delta;

    collisions = getCollisions(objs);
    collisionCount += collisions.size();
    while (collisions.size() && System::Update()) {
      delta = calcCollisions(objs, collisions, delta);
      collisions = getCollisions(objs);
      collisionCount += collisions.size();

      drawAll(objs, font);
    }

    drawAll(objs, font);
  }
}
