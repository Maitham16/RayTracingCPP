// moving sphere header class
#ifndef MOVING_SPHERE_H
#define MOVING_SPHERE_H

#include "Hitable.h"
#include "AABB.h"

class Moving_sphere : public Hitable
{
public:
    Vec3 center0, center1;
    double time0, time1;
    double radius;
    Material *material;

    Moving_sphere() {}
    Moving_sphere(Vec3 cen0, Vec3 cen1, double t0, double t1, double r, Material *m) : center0(cen0), center1(cen1), time0(t0), time1(t1), radius(r), material(m) {}

    // override the pure virtual function in Hitable class
    virtual bool intersect(const Ray &ray, float t_min, float t_max, Hit_record &hit_record) const;
    Vec3 center(double time) const;

    virtual bool bounding_box(float tm0, float tm1, AABB &box) const;
};

Vec3 Moving_sphere::center(double time) const
{
    return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
}

bool Moving_sphere::intersect(const Ray &ray, float t_min, float t_max, Hit_record &hit_record) const
{
    Vec3 oc = ray.origin() - center(ray.time());
    float a = dot(ray.direction(), ray.direction());
    float b = dot(oc, ray.direction());
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - a * c;
    if (discriminant > 0)
    {
        float temp = (-b - sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min)
        {
            hit_record.t = temp;
            hit_record.p = ray.point_at(hit_record.t);
            hit_record.normal = (hit_record.p - center(ray.time())) / radius;
            hit_record.material = material;
            return true;
        }
        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min)
        {
            hit_record.t = temp;
            hit_record.p = ray.point_at(hit_record.t);
            hit_record.normal = (hit_record.p - center(ray.time())) / radius;
            hit_record.material = material;
            return true;
        }
    }
    return false;
};

bool Moving_sphere::bounding_box(float tm0, float tm1, AABB &box) const
{
    AABB box0(center(tm0) - Vec3(radius, radius, radius), center(tm0) + Vec3(radius, radius, radius));
    AABB box1(center(tm1) - Vec3(radius, radius, radius), center(tm1) + Vec3(radius, radius, radius));
    box = surrounding_box(box0, box1);
    return true;
};

#endif