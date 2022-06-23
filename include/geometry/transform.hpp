#pragma once

#include "geometry/object3d.hpp"

class Transform : public Object3D {
private:
    Object3D *obj;
    Matrix4f trans;

public:
    Transform() = default;

    Transform(const Matrix4f &_trans, Object3D *o) {
        obj = o;
        trans = _trans.inverse();
    }

    virtual ~Transform() override = default;

    virtual bool intersect(const Ray &r, Hit &h, double tmin) const override {
		Vector3f trSource = (trans * Vector4f(r.o, 1)).xyz();
		Vector3f trDirection = (trans * Vector4f(r.d, 0)).xyz();
		Ray tr(trSource, trDirection);
		bool inter = obj->intersect(tr, h, tmin);
		if (inter)
			h.surface = HitSurface(
                r.at(h.t),
				(trans.transposed() * Vector4f(h.surface.normal, 0)).xyz().normalized(),
				(trans.transposed() * Vector4f(h.surface.geoNormal, 0)).xyz().normalized(),
				h.surface.cord,
			    h.surface.hasTexture
            );
		return inter;
	}

	std::pair<HitSurface, double> samplePoint(RandomEngine &reng) const override {
		auto s = obj->samplePoint(reng);
		return std::make_pair(HitSurface {
            (trans.inverse() * Vector4f(s.first.position, 1)).xyz(),
            (trans.transposed() * Vector4f(s.first.normal, 0)).xyz().normalized()
        }, s.second);
	}
};