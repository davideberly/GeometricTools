// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.26

#pragma once

#include <Mathematics/Matrix3x3.h>
#include <Mathematics/Rotation.h>
#include <functional>

namespace gte
{
    // The rigid body state is stored in a separate structure so that
    // the Force and Torque functionals can be passed a single object
    // to avoid a large number of parameters that would otherwise have
    // to be passed to the functionals. This makes the Runge-Kutta ODE
    // solver easier to read. The RigidBody<T> class provides wrappers
    // around the state accessors to avoid exposing a public state member.
    template <typename T>
    class RigidBodyState
    {
    public:
        RigidBodyState()
            :
            mMass(static_cast<T>(0)),
            mInvMass(static_cast<T>(0)),
            mBodyInertia(Matrix3x3<T>::Zero()),
            mBodyInverseInertia(Matrix3x3<T>::Zero()),
            mPosition(Vector3<T>::Zero()),
            mQOrientation(Quaternion<T>::Identity()),
            mLinearMomentum(Vector3<T>::Zero()),
            mAngularMomentum(Vector3<T>::Zero()),
            mWorldInertia(Matrix3x3<T>::Zero()),
            mWorldInverseInertia(Matrix3x3<T>::Zero()),
            mROrientation(Matrix3x3<T>::Identity()),
            mLinearVelocity(Vector3<T>::Zero()),
            mAngularVelocity(Vector3<T>::Zero()),
            mQAngularVelocity{}
        {
        }

        // Set the mass to a positive number for movable bodies. Set the mass
        // to zero for immovable objects. A body is immovable in the physics
        // simulation, but you can position and orient the immovable body
        // manually, typically during the creation of the physics objects.
        void SetMass(T mass)
        {
            T const zero = static_cast<T>(0);

            if (mass > zero)
            {
                mMass = mass;
                mInvMass = static_cast<T>(1) / mass;
            }
            else
            {
                mMass = zero;
                mInvMass = zero;
            }
        }

        // Set the body inertia to a positive definite matrix for movable
        // bodies. Set the body inertia to the zero matrix for immovable
        // objects, but you can position and orient the immovable body
        // manually, typically during the creation of the physics objects.
        void SetBodyInertia(Matrix3x3<T> const& bodyInertia)
        {
            Matrix3x3<T> const zero = Matrix3x3<T>::Zero();

            if (bodyInertia != zero)
            {
                mBodyInertia = bodyInertia;
                mBodyInverseInertia = Inverse(bodyInertia);
                UpdateWorldInertialQuantities();
            }
            else
            {
                mBodyInertia = zero;
                mBodyInverseInertia = zero;
                mWorldInertia = zero;
                mWorldInverseInertia = zero;
            }
        }

        inline bool IsMovable() const
        {
            return mMass > static_cast<T>(0);
        }

        inline bool IsImmovable() const
        {
            return mMass == static_cast<T>(0);
        }

        inline void SetPosition(Vector3<T> const& position)
        {
            mPosition = position;
        }

        void SetQOrientation(Quaternion<T> const& qOrientation, bool normalize = false)
        {
            mQOrientation = qOrientation;
            if (normalize)
            {
                Normalize(mQOrientation);
            }

            mROrientation = Rotation<3, T>(qOrientation);
            if (IsMovable())
            {
                UpdateWorldInertialQuantities();
            }
        }

        void SetLinearMomentum(Vector3<T> const& linearMomentum)
        {
            if (IsMovable())
            {
                mLinearMomentum = linearMomentum;
                mLinearVelocity = mInvMass * linearMomentum;
            }
        }

        void SetAngularMomentum(Vector3<T> const& angularMomentum)
        {
            if (IsMovable())
            {
                mAngularMomentum = angularMomentum;
                mAngularVelocity = mWorldInverseInertia * angularMomentum;
                mQAngularVelocity[0] = mAngularVelocity[0];
                mQAngularVelocity[1] = mAngularVelocity[1];
                mQAngularVelocity[2] = mAngularVelocity[2];
                mQAngularVelocity[3] = static_cast<T>(0);
            }
        }

        void SetROrientation(Matrix3x3<T> const& rOrientation)
        {
            mROrientation = rOrientation;
            mQOrientation = Rotation<3, T>(rOrientation);
            if (IsMovable())
            {
                UpdateWorldInertialQuantities();
            }
        }

        void SetLinearVelocity(Vector3<T> const& linearVelocity)
        {
            if (IsMovable())
            {
                mLinearVelocity = linearVelocity;
                mLinearMomentum = mMass * linearVelocity;
            }
        }

        void SetAngularVelocity(Vector3<T> const& angularVelocity)
        {
            if (IsMovable())
            {
                mAngularVelocity = angularVelocity;
                mAngularMomentum = mWorldInertia * angularVelocity;
                mQAngularVelocity[0] = mAngularVelocity[0];
                mQAngularVelocity[1] = mAngularVelocity[1];
                mQAngularVelocity[2] = mAngularVelocity[2];
                mQAngularVelocity[3] = static_cast<T>(0);
            }
        }

        inline T const& GetMass() const
        {
            return mMass;
        }

        inline T const& GetInverseMass() const
        {
            return mInvMass;
        }

        inline Matrix3x3<T> const& GetBodyInertia() const
        {
            return mBodyInertia;
        }

        inline Matrix3x3<T> const& GetBodyInverseInertia() const
        {
            return mBodyInverseInertia;
        }

        inline Matrix3x3<T> const& GetWorldInertia() const
        {
            return mWorldInertia;
        }

        inline Matrix3x3<T> const& GetWorldInverseInertia() const
        {
            return mWorldInverseInertia;
        }

        inline Vector3<T> const& GetPosition() const
        {
            return mPosition;
        }

        inline Quaternion<T> const& GetQOrientation() const
        {
            return mQOrientation;
        }

        inline Vector3<T> const& GetLinearMomentum() const
        {
            return mLinearMomentum;
        }

        inline Vector3<T> const& GetAngularMomentum() const
        {
            return mAngularMomentum;
        }

        inline Matrix3x3<T> const& GetROrientation() const
        {
            return mROrientation;
        }

        inline Vector3<T> const& GetLinearVelocity() const
        {
            return mLinearVelocity;
        }

        inline Vector3<T> const& GetAngularVelocity() const
        {
            return mAngularVelocity;
        }

        inline Quaternion<T> const& GetQAngularVelocity() const
        {
            return mQAngularVelocity;
        }

    private:
        void UpdateWorldInertialQuantities()
        {
            mWorldInertia = MultiplyABT(
                mROrientation * mBodyInertia, mROrientation);

            mWorldInverseInertia = MultiplyABT(
                mROrientation * mBodyInverseInertia, mROrientation);
        }

        // Constant quantities during the simulation.
        T mMass;
        T mInvMass;
        Matrix3x3<T> mBodyInertia;
        Matrix3x3<T> mBodyInverseInertia;

        // State variables in the differential equations of motion.
        Vector3<T> mPosition;
        Quaternion<T> mQOrientation;
        Vector3<T> mLinearMomentum;
        Vector3<T> mAngularMomentum;

        // Quantities derived from the state variables.
        Matrix3x3<T> mWorldInertia;
        Matrix3x3<T> mWorldInverseInertia;
        Matrix3x3<T> mROrientation;
        Vector3<T> mLinearVelocity;
        Vector3<T> mAngularVelocity;
        Quaternion<T> mQAngularVelocity;
    };

    template <typename T>
    class RigidBody
    {
    public:
        // The rigid body state is initialized to zero values. Set the members
        // before starting the simulation. For immovable objects, set mass to
        // zero.
        RigidBody()
            :
            Force{},
            Torque{},
            mState{}
        {
        }

        virtual ~RigidBody() = default;

        // Set the mass to a positive number for movable bodies. Set the mass
        // to zero for immovable objects. A body is immovable in the physics
        // simulation, but you can position and orient the immovable body
        // manually, typically during the creation of the physics objects.
        inline void SetMass(T mass)
        {
            mState.SetMass(mass);
        }

        // Set the body inertia to a positive definite matrix for movable
        // bodies. Set the body inertia to the zero matrix for immovable
        // objects, but you can position and orient the immovable body
        // manually, typically during the creation of the physics objects.
        inline void SetBodyInertia(Matrix3x3<T> const& bodyInertia)
        {
            mState.SetBodyInertia(bodyInertia);
        }

        inline bool IsMovable() const
        {
            return mState.IsMovable();
        }

        inline bool IsImmovable() const
        {
            return mState.IsImmovable();
        }

        inline void SetPosition(Vector3<T> const& position)
        {
            mState.SetPosition(position);
        }

        inline void SetQOrientation(Quaternion<T> const& qOrientation, bool normalize = false)
        {
            mState.SetQOrientation(qOrientation, normalize);
        }

        inline void SetLinearMomentum(Vector3<T> const& linearMomentum)
        {
            mState.SetLinearMomentum(linearMomentum);
        }

        inline void SetAngularMomentum(Vector3<T> const& angularMomentum)
        {
            mState.SetAngularMomentum(angularMomentum);
        }

        inline void SetROrientation(Matrix3x3<T> const& rOrientation)
        {
            mState.SetROrientation(rOrientation);
        }

        inline void SetLinearVelocity(Vector3<T> const& linearVelocity)
        {
            mState.SetLinearMomentum(linearVelocity);
        }

        inline void SetAngularVelocity(Vector3<T> const& angularVelocity)
        {
            mState.SetAngularVelocity(angularVelocity);
        }

        inline T const& GetMass() const
        {
            return mState.GetMass();
        }

        inline T const& GetInverseMass() const
        {
            return mState.GetInverseMass();
        }

        inline Matrix3x3<T> const& GetBodyInertia() const
        {
            return mState.GetBodyInertia();
        }

        inline Matrix3x3<T> const& GetBodyInverseInertia() const
        {
            return mState.GetBodyInverseInertia();
        }

        inline Matrix3x3<T> const& GetWorldInertia() const
        {
            return mState.GetWorldInertia();
        }

        inline Matrix3x3<T> const& GetWorldInverseInertia() const
        {
            return mState.GetWorldInverseInertia();
        }

        inline Vector3<T> const& GetPosition() const
        {
            return mState.GetPosition();
        }

        inline Quaternion<T> const& GetQOrientation() const
        {
            return mState.GetQOrientation();
        }

        inline Vector3<T> const& GetLinearMomentum() const
        {
            return mState.GetLinearMomentum();
        }

        inline Vector3<T> const& GetAngularMomentum() const
        {
            return mState.GetAngularMomentum();
        }

        inline Matrix3x3<T> const& GetROrientation() const
        {
            return mState.GetROrientation();
        }

        inline Vector3<T> const& GetLinearVelocity() const
        {
            return mState.GetLinearVelocity();
        }

        inline Vector3<T> const& GetAngularVelocity() const
        {
            return mState.GetAngularVelocity();;
        }

        inline Quaternion<T> const& GetQAngularVelocity() const
        {
            return mState.GetQAngularVelocity();
        }

        // Force and torque functions. The first input (type T) is the
        // simulation time. The second input is rigid body state. These
        // functions must be set before starting the simulation.
        using Function = std::function<Vector3<T>(T, RigidBodyState<T> const&)>;
        Function Force;
        Function Torque;

        // Runge-Kutta fourth-order differential equation solver
        void Update(T const& t, T const& dt)
        {
            // TODO: When GTE_MAT_VEC is not defined (i.e. use vec-mat),
            // test to see whether dq/dt = 0.5 * w * q (mat-vec convention)
            // needs to become a different equation.

            T const half = static_cast<T>(0.5);
            T const two = static_cast<T>(2);
            T const six = static_cast<T>(6);

            T halfDT = half * dt;
            T sixthDT = dt / six;
            T TpHalfDT = t + halfDT;
            T TpDT = t + dt;

            RigidBodyState<T> newState{};
            newState.SetMass(GetMass());
            newState.SetBodyInertia(GetBodyInertia());

            // A1 = G(T,S0), B1 = S0 + (DT/2)*A1
            Vector3<T> A1DXDT = GetLinearVelocity();
            Quaternion<T> W = GetQAngularVelocity();
            Quaternion<T> A1DQDT = half * W * GetQOrientation();
            Vector3<T> A1DPDT = Force(t, mState);
            Vector3<T> A1DLDT = Torque(t, mState);
            newState.SetPosition(GetPosition() + halfDT * A1DXDT);
            newState.SetQOrientation(GetQOrientation() + halfDT * A1DQDT, true);
            newState.SetLinearMomentum(GetLinearMomentum() + halfDT * A1DPDT);
            newState.SetAngularMomentum(GetAngularMomentum() + halfDT * A1DLDT);

            // A2 = G(T+DT/2,B1), B2 = S0 + (DT/2)*A2
            Vector3<T> A2DXDT = newState.GetLinearVelocity();
            W = newState.GetQAngularVelocity();
            Quaternion<T> A2DQDT = half * W * newState.GetQOrientation();
            Vector3<T> A2DPDT = Force(TpHalfDT, newState);
            Vector3<T> A2DLDT = Torque(TpHalfDT, newState);
            newState.SetPosition(GetPosition() + halfDT * A2DXDT);
            newState.SetQOrientation(GetQOrientation() + halfDT * A2DQDT, true);
            newState.SetLinearMomentum(GetLinearMomentum() + halfDT * A2DPDT);
            newState.SetAngularMomentum(GetAngularMomentum() + halfDT * A2DLDT);

            // A3 = G(T+DT/2,B2), B3 = S0 + DT*A3
            Vector3<T> A3DXDT = newState.GetLinearVelocity();
            W = newState.GetQAngularVelocity();
            Quaternion<T> A3DQDT = half * W * newState.GetQOrientation();
            Vector3<T> A3DPDT = Force(TpHalfDT, newState);
            Vector3<T> A3DLDT = Torque(TpHalfDT, newState);
            newState.SetPosition(GetPosition() + dt * A3DXDT);
            newState.SetQOrientation(GetQOrientation() + dt * A3DQDT, true);
            newState.SetLinearMomentum(GetLinearMomentum() + dt * A3DPDT);
            newState.SetAngularMomentum(GetAngularMomentum() + dt * A3DLDT);

            // A4 = G(T+DT,B3), S1 = S0 + (DT/6)*(A1+2*(A2+A3)+A4)
            Vector3<T> A4DXDT = newState.GetLinearVelocity();
            W = newState.GetQAngularVelocity();
            Quaternion<T> A4DQDT = half * W * newState.GetQOrientation();
            Vector3<T> A4DPDT = Force(TpDT, newState);
            Vector3<T> A4DLDT = Torque(TpDT, newState);

            SetPosition(GetPosition() +
                sixthDT * (A1DXDT + two * (A2DXDT + A3DXDT) + A4DXDT));

            SetQOrientation(GetQOrientation() +
                sixthDT * (A1DQDT + two * (A2DQDT + A3DQDT) + A4DQDT), true);

            SetLinearMomentum(GetLinearMomentum() +
                sixthDT * (A1DPDT + two * (A2DPDT + A3DPDT) + A4DPDT));

            SetAngularMomentum(GetAngularMomentum() +
                sixthDT * (A1DLDT + two * (A2DLDT + A3DLDT) + A4DLDT));
        }

    private:
        RigidBodyState<T> mState;
    };
}
