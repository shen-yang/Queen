#ifndef BoundingBox_h__
#define BoundingBox_h__

#include "Math.hpp"
#include "Vector.hpp"

namespace RxLib {

template <typename Real>
class BoundingBox
{
public:
	typedef Real value_type;

public:
	BoundingBox() : Defined(false) { }

	BoundingBox(const Vector<Real,3>& min, const Vector<Real,3>& max)
		: Min(min), Max(max), Defined(true) { }

	BoundingBox(const BoundingBox<Real>& rhs)
		: Min(rhs.Min), Max(rhs.Max), Defined(rhs.Defined) { }

	BoundingBox& operator = (const BoundingBox<Real>& rhs)
	{
		if (this != &rhs)
		{
			Max = rhs.Max;
			Min = rhs.Min;
			Defined = rhs.Defined;
		}
		return *this;
	}

	bool operator == ( const BoundingBox<Real>& rhs)
	{
		return (Min == rhs.Min) && (Max == rhs.Max);
	}

	/**
	 * Reset the bounding box to undefined.
	 */
	void SetNull()	{ Defined = false; }

	/**
	 * Return center of box.
	 */
	Vector<Real,3> Center() const { return (Min + Max) * Real(0.5); }

	/**
	 * Merge a point
	 */
	void Merge( const Vector<Real,3>& point )
	{
		if (!Defined)
		{
			Max = Min = point;
			Defined = true;
			return;
		}

		if (point.X() < Min.X())	Min.X() = point.X();
		if (point.Y() < Min.Y())	Min.Y() = point.Y();
		if (point.Z() < Min.Z())	Min.X() = point.Z();

		if (point.X() > Max.X())	Max.X() = point.X();
		if (point.Y() > Max.Y())	Max.Y() = point.Y();
		if (point.Z() > Max.Z())	Max.X() = point.Z();
	}
	
	/**
	 * Merge another box
	 */
	void Merge( const BoundingBox<Real>& box )
	{
		// do nothing if the rhs box id undefined.
		if (!box.Defined)
		{
			return;
		}
		// Otherwise if current null, just take rhs box
		else if (!Defined)
		{
			Min = box.Min;
			Max = box.Max;
			Defined = true;
			return;
		}
		// Otherwise merge
		else
		{
			if (box.Min.X() < Min.X())	Min.X() = box.Min.X();
			if (box.Min.Y() < Min.Y())	Min.Y() = box.Min.Y();
			if (box.Min.Z() < Min.Z())	Min.X() = box.Min.Z();

			if (box.Max.X() > Max.X())	Max.X() = box.Max.X();
			if (box.Max.Y() > Max.Y())	Max.Y() = box.Max.Y();
			if (box.Max.Z() > Max.Z())	Max.X() = box.Max.Z();
		}
	}

	/**
	 * Determines whether contains the specified box.
	 */
	ContainmentType Contains( const BoundingBox<Real>& box )
	{
		if( Max.X() < box.Min.X() || Min.X() > box.Max.X() )
			return CT_Disjoint;

		if( Max.Y() < box.Min.Y() || Min.Y() > box.Max.Y() )
			return CT_Disjoint;

		if( Max.Z() < box.Min.Z() || Min.Z() > box.Max.Z() )
			return CT_Disjoint;

		if( Min.X() <= box.Min.X() && box.Max.X() <= Max.X() && 
			Min.Y() <= box.Min.Y() && box.Max.Y() <= Max.Y() &&
			Min.Z() <= box.Min.Z() && box.Max.Z() <= Max.Z() )
			return CT_Contains;

		return CT_Intersects;
	}

	/**
	 * Determines whether contains the specified point.
	 */
	ContainmentType Contains( const Vector<Real,3>& point )
	{
		if( Min.X() <= point.X() && point.X() <= Max.X() && Min.Y() <= point.Y() && 
			point.Y() <= Max.Y() && Min.Z() <= point.Z() && point.Z() <= Max.Z() )
			return CT_Contains;

		return CT_Disjoint;
	}

	/**
	 * Determines whether a sphere intersects the specified object.
	 */
	bool Intersects( const BoundingBox<Real>& box )
	{
		if ( Max.X() < box.Min.X() || Min.X() > box.Max.X() )
			return false;

		if ( Max.Y() < box.Min.Y() || Min.Y() > box.Max.Y() )
			return false;

		return ( Max.Z() >= box.Min.Z() && Min.Z() <= box.Max.Z() );
	}


public:
	Vector<Real,3> Max;
	Vector<Real,3> Min;
	bool Defined;
};


template<typename Real>
BoundingBox<Real> Merge( const BoundingBox<Real>& box1, const BoundingBox<Real>& box2 )
{
	Vector<Real,3> min, max;

	if (box1.Min.X() < box2.Min.X())	min.X() = box.Min.X();
	if (box1.Min.Y() < box2.Min.Y())	min.Y() = box.Min.Y();
	if (box1.Min.Z() < box2.Min.Z())	min.X() = box.Min.Z();

	if (box1.Max.X() > box2.Max.X())	max.X() = box.Max.X();
	if (box1.Max.Y() > box2.Max.Y())	max.Y() = box.Max.Y();
	if (box1.Max.Z() > box2.Max.Z())	max.X() = box.Max.Z();

	return BoundingBox<Real>(min, max);
}

typedef BoundingBox<float> BoundingBoxf;
typedef BoundingBox<double> BoundingBoxd;

}

#endif // BoundingBox_h__
