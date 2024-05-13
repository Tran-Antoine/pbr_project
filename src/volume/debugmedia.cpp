#include <core/common.h>
#include <volume/medium.h>
#include <volume/coefficient.h>

NORI_NAMESPACE_BEGIN

class Debug0Coeff : public MediumCoefficient {
public:
    explicit Debug0Coeff(float coeff1, float coeff2) : coeff1(coeff1), coeff2(coeff2) {}

    Color3f eval(const Point3f& p, const Vector3f& v) const override {
        if(p.x() < 0) return coeff1;
        return coeff2;
    }
    float maj(const Ray3f& ray) const override { return coeff1 > coeff2 ? coeff1 : coeff2; }
private:
    float coeff1;
    float coeff2;
};

class Debug1Coeff : public MediumCoefficient {
public:
    explicit Debug1Coeff() {}

    Color3f eval(const Point3f& p, const Vector3f& v) const override {
        return p.x() + 2.f;
    }
    float maj(const Ray3f& ray) const override { return 3.f; }

};

class Debug2Coeff : public MediumCoefficient {
public:
    explicit Debug2Coeff() {}

    Color3f eval(const Point3f& p, const Vector3f& v) const override {
        if(p.norm() < 0.5) {
            return 0.f;
        }

        if(abs(p.y()) > 0.5 && abs(p.x()) > 0.5) {
            return 0.f;
        }

        return 1.f;
    }
    float maj(const Ray3f& ray) const override { return 1.f; }

};

class Debug0Medium : public Medium {

public:
    explicit Debug0Medium(const PropertyList &props) {
        phase = new UniformPhase();
        //absorption = new VoxelReader(props.getString("voxel_path"), trafo, props.getFloat("d_factor", 1.f));
        absorption = new Debug0Coeff(1.f, 2.0f);
        scattering = new ConstantCoefficient(0.f);
    }

    std::string toString() const override {
        return "voxel";
    }

    BoundingBox3f bounds() const override {
        return BoundingBox3f(Point3f(-1,-1,-1), Point3f(1,1,1));
    }
};

class Debug1Medium : public Medium {

public:
    explicit Debug1Medium(const PropertyList &props) {
        phase = new UniformPhase();
        //absorption = new VoxelReader(props.getString("voxel_path"), trafo, props.getFloat("d_factor", 1.f));
        absorption = new Debug1Coeff();
        scattering = new ConstantCoefficient(0.f);
    }

    std::string toString() const override {
        return "voxel";
    }

    BoundingBox3f bounds() const override {
        return BoundingBox3f(Point3f(-1,-1,-1), Point3f(1,1,1));
    }
};

class Debug2Medium : public Medium {

public:
    explicit Debug2Medium(const PropertyList &props) {
        phase = new UniformPhase();
        //absorption = new VoxelReader(props.getString("voxel_path"), trafo, props.getFloat("d_factor", 1.f));
        absorption = new Debug2Coeff();
        scattering = new ConstantCoefficient(0.f);
    }

    std::string toString() const override {
        return "voxel";
    }

    BoundingBox3f bounds() const override {
        return BoundingBox3f(Point3f(-1,-1,-1), Point3f(1,1,1));
    }
};

class Debug3Medium : public Medium {

public:
    explicit Debug3Medium(const PropertyList &props) {
        phase = new UniformPhase();
        //absorption = new VoxelReader(props.getString("voxel_path"), trafo, props.getFloat("d_factor", 1.f));
        absorption = new ConstantCoefficient(5);
        scattering = new ConstantCoefficient(0.f);
    }

    std::string toString() const override {
        return "voxel";
    }

    BoundingBox3f bounds() const override {
        return BoundingBox3f(Point3f(-1,-1,-1), Point3f(1,1,1));
    }
};

class Debug4Medium : public Medium {

public:
    explicit Debug4Medium(const PropertyList &props) {
        phase = new UniformPhase();
        //absorption = new VoxelReader(props.getString("voxel_path"), trafo, props.getFloat("d_factor", 1.f));
        absorption = new ConstantCoefficient(Color3f(0.01, 0.0001, 0.01));
        scattering = new ConstantCoefficient(Color3f(0.3, 0.0001, 0.3));
    }

    std::string toString() const override {
        return "voxel";
    }

    BoundingBox3f bounds() const override {
        return BoundingBox3f(Point3f(-10,20,-5), Point3f(50,50,50));
    }
};

NORI_REGISTER_CLASS(Debug0Medium, "debug0");
NORI_REGISTER_CLASS(Debug1Medium, "debug1");
NORI_REGISTER_CLASS(Debug2Medium, "debug2");
NORI_REGISTER_CLASS(Debug3Medium, "debug3");
NORI_REGISTER_CLASS(Debug4Medium, "debug4");

NORI_NAMESPACE_END