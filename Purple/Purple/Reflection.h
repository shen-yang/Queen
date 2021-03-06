#ifndef Reflection_h__
#define Reflection_h__

#include "Prerequisites.h"
#include <Math.hpp>

namespace Purple {

inline float CosTheta(const float3& w) 
{
	return w.Z();
}

inline float AbsCosTheta(const float3& w) 
{
	return fabsf(w.Z());
}

inline float SinTheta2(const float3& w) 
{
	return (std::max)(0.0f, 1.0f - w.Z() * w.Z());
}

inline float SinTheta(const float3& w) 
{
	return sqrtf(SinTheta2(w));
}

inline float SinPhi(const float3& w) 
{
	float sintheta = SinTheta(w);
	if (sintheta == 0.f) return 1.f;
	return RxLib::Clamp(w.Y() / sintheta, -1.f, 1.f);
}

inline float CosPhi(const float3& w) 
{
	float sintheta = SinTheta(w);
	if (sintheta == 0.f) return 0.f;
	return RxLib::Clamp(w.X() / sintheta, -1.f, 1.f);
}

inline bool SameHemisphere(const float3& w, const float3& wp) 
{
	return w.Z() * wp.Z() > 0.f;
}

inline float3 SphericalDirection(float cosTheta, float sinTheta, float phi)
{
	return float3(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
}

struct Fresnel
{
	virtual ~Fresnel() { };
	virtual ColorRGB Evaluate(float cosi) const = 0;
};

class BxDF 
{
public:
	
	enum BSDFType
	{
		BSDF_Reflection        = 1U << 0,
		BSDF_Transmission      = 1U << 1,
		BSDF_Diffuse           = 1U << 2,
		BSDF_Glossy            = 1U << 3,
		BSDF_Specular          = 1U << 4,
		BSDF_All_Type          = BSDF_Diffuse | BSDF_Glossy | BSDF_Specular,
		BSDF_All_Reflection    = BSDF_Reflection | BSDF_All_Type,
		BSDF_All_Transmission  = BSDF_Transmission | BSDF_All_Type,
		BSDF_All               = BSDF_All_Reflection | BSDF_All_Transmission
	};


public:

	BxDF(uint32_t bxdfType) : BxDFType(bxdfType) { }
	virtual ~BxDF() { }

	/**
	 * @breif Evaluate the BRDF for a pair of given directions.
	 */
	virtual ColorRGB f(const float3& wo, const float3& wi) const = 0;

	/**
	 * Sample the BRDF and return the sampled BRDF value. Also return the sampled indicent 
	 */
	virtual ColorRGB Sample_f(const float3& wo, float3* wi, float u1, float u2, float* pdf) const;

	/**
	 * 计算给定方向的光在半球空间反射的积分，rho <= 1 能量守恒。如果不能通过解析方法积分。就根据
	 * Samples 用Mento Carlo积分的方法计算。
	 */
	virtual ColorRGB rho(const float3& wo, int32_t numSamples, const float* samples) const;

	/**
	 * 
	 */
	virtual ColorRGB rho(int32_t numSamples, const float* samples1, const float* samples2) const;
	
	virtual float Pdf(const float3& wo, const float3& wi) const;


public:
	const uint32_t BxDFType;
};

/**
  * Perfect specular reflection
  */
class SpecularRelection : public BxDF
{
public:
	SpecularRelection(const ColorRGB& reflectance, Fresnel* fresnel)
		: BxDF(BSDF_Reflection | BSDF_Specular), mR(reflectance), mFresnel(fresnel)
	{

	}

	ColorRGB f(const float3& wo, const float3& wi) const  { return ColorRGB::Black; }

	float Pdf(const float3& wo, const float3& wi) const { return 0.0f; }

	/**
	 * this computes wi: the direction of perfect mirror reflection
	 */
	ColorRGB Sample_f(const float3& wo, float3* wi, float u1, float u2, float* pdf) const;

private:
	ColorRGB mR;			// reflect color
	Fresnel* mFresnel;	    
};

class Lambertian : public BxDF
{
public:
	Lambertian(const ColorRGB& reflectance) 
		: BxDF(BSDF_Reflection | BSDF_Diffuse), mR(reflectance) { }

	ColorRGB f(const float3& wo, const float3& wi) const	{ return mR / RxLib::Mathf::INV_PI; }

	ColorRGB rho(const float3& wo, int32_t numSamples, const float* samples) const { return mR; }

	ColorRGB rho(int32_t numSamples, const float* samples1, const float* samples2) const { return mR; }

private:
	ColorRGB mR;			// reflect color
};

class OrenNayar : public BxDF
{
public:
	OrenNayar(const ColorRGB& reflectance, float sigma)
		: BxDF(BSDF_Reflection | BSDF_Diffuse), mR(reflectance)
	{
		sigma = RxLib::ToRadian(sigma);
		float sigma2 = sigma*sigma; 
		
		A = 1.0f - sigma2 / (2.0f * sigma2 + 0.33f);
		B = 0.45f * sigma2 / (sigma2 + 0.09f);
	}

	ColorRGB f(const float3& wo, const float3& wi) const;

private:
	float A, B;             // OrenNayar BRDF contants
	ColorRGB mR;			// reflect color
};


struct MicrofacetDistribution
{
	virtual ~MicrofacetDistribution() {}
	virtual float D(const float3& wh) const = 0;
	virtual void Sample_f(const float3& wo, float3* wi, float u1, float u2, float* pdf) const = 0;
	virtual float Pdf(const float3& wo, const float3& wi) const = 0;
};

class TorranceSparrow : public BxDF
{
public:
	TorranceSparrow(const ColorRGB& reflectance, Fresnel* fresnel)
		: BxDF(BSDF_Reflection | BSDF_Glossy), mR(reflectance), mFresnel(fresnel) {}

	ColorRGB f(const float3& wo, const float3& wi) const;

	float Pdf(const float3& wo, const float3& wi) const;

	ColorRGB Sample_f(const float3& wo, float3* wi, float u1, float u2, float* pdf) const;

private:

	/**
	 * Geometric attenuation term
	 */
	float G(const float3& wo, const float3& wi, const float3& wh) const;

private:

	ColorRGB mR;
	Fresnel* mFresnel;	 
	MicrofacetDistribution* mD;
};

struct Blin : MicrofacetDistribution
{
	Blin(float e) : mExponent(e) {}
	
	float D(const float3& wh) const
	{
		return (mExponent + 2.0f) * RxLib::Mathf::INV_TWO_PI * powf(AbsCosTheta(wh), mExponent);
	}

	void Sample_f(const float3& wo, float3* wi, float u1, float u2, float* pdf) const;
	float Pdf(const float3& wo, const float3& wi) const;

private:
	float mExponent;
};


}


#endif // Reflection_h__
