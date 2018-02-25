#ifndef POINT_H
#define POINT_H

#include <cmath>

template <typename N>
struct Point {
    private:
    N x = 0, y = 0;

    public:
    N getX() const {return x;}
    N getY() const {return y;}

    void setX(N n) {x = n;}
    void setY(N n) {y = n;}

    /* Why do I need a getDistance function you ask?
    Hahahaha idk */
    template <typename R = N, typename M>
    R getDistance(const Point<M>& anotherPoint) {
        return static_cast<R>(std::sqrt(pow(getX() - anotherPoint.getX() , 2) 
                            + pow(getY() - anotherPoint.getY() , 2)));
    }

    template <typename R = N, typename M>
    R getGradient(const Point<M>& anotherPoint) {
        return static_cast<R>((getY() - anotherPoint(getY())) / (getX() - anotherPoint(getX())));
    }
};

#endif