//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsVector2.h"
#include "BsVector3.h"
#include "BsVector4.h"
#include "BsMatrix3.h"
#include "BsMatrix4.h"
#include "BsMatrixNxM.h"
#include "BsVectorNI.h"
#include "BsColor.h"

namespace BansheeEngine
{
	/** @addtogroup Implementation
	 *  @{
	 */

	template<bool Core> struct TGpuParamsPtrType { };
	template<> struct TGpuParamsPtrType<false> { typedef SPtr<GpuParams> Type; };
	template<> struct TGpuParamsPtrType<true> { typedef SPtr<GpuParamsCore> Type; };

	template<bool Core> struct TGpuParamTextureType { };
	template<> struct TGpuParamTextureType < false > { typedef HTexture Type; };
	template<> struct TGpuParamTextureType < true > { typedef SPtr<TextureCore> Type; };

	template<bool Core> struct TGpuParamSamplerStateType { };
	template<> struct TGpuParamSamplerStateType < false > { typedef SPtr<SamplerState> Type; };
	template<> struct TGpuParamSamplerStateType < true > { typedef SPtr<SamplerStateCore> Type; };

	template<bool Core> struct TGpuParamBufferType { };
	template<> struct TGpuParamBufferType < false > { typedef SPtr<GpuParamBlockBuffer> Type; };
	template<> struct TGpuParamBufferType < true > { typedef SPtr<GpuParamBlockBufferCore> Type; };

	/**
	 * Policy class that allows us to re-use this template class for matrices which might need transposing, and other 
	 * types which do not. Matrix needs to be transposed for certain render systems depending on how they store them 
	 * in memory.
	 */
	template<class Type>
	struct TransposePolicy
	{
		static Type transpose(const Type& value) { return value; }
		static bool transposeEnabled(bool enabled) { return false; }
	};

	/** Transpose policy for 3x3 matrix. */
	template<>
	struct TransposePolicy<Matrix3>
	{
		static Matrix3 transpose(const Matrix3& value) { return value.transpose(); }
		static bool transposeEnabled(bool enabled) { return enabled; }
	};

	/**	Transpose policy for 4x4 matrix. */
	template<>
	struct TransposePolicy<Matrix4>
	{
		static Matrix4 transpose(const Matrix4& value) { return value.transpose(); }
		static bool transposeEnabled(bool enabled) { return enabled; }
	};

	/**	Transpose policy for NxM matrix. */
	template<int N, int M>
	struct TransposePolicy<MatrixNxM<N, M>>
	{
		static MatrixNxM<N, M> transpose(const MatrixNxM<N, M>& value) { return value.transpose(); }
		static bool transposeEnabled(bool enabled) { return enabled; }
	};

	/**
	 * A handle that allows you to set a GpuProgram parameter. Internally keeps a reference to the GPU parameter buffer and
	 * the necessary offsets. You should specialize this type for specific parameter types. 
	 *
	 * Object of this type must be returned by a Material. Setting/Getting parameter values will internally access a GPU 
	 * parameter buffer attached to the Material this parameter was created from. Anything rendered with that material will
	 * then use those set values.
	 * 			
	 * @note	
	 * Normally you can set a GpuProgram parameter by calling various set/get methods on a Material. This class primarily 
	 * used an as optimization in performance critical bits of code where it is important to locate and set parameters 
	 * quickly without any lookups (Mentioned set/get methods expect a parameter name). You just retrieve the handle once 
	 * and then set the parameter value many times with minimal performance impact.
	 * 
	 * @see		Material
	 */
	template<class T, bool Core>
	class BS_CORE_EXPORT TGpuDataParam
	{
	private:
		typedef typename TGpuParamBufferType<Core>::Type GpuParamBufferType;
		typedef typename TGpuParamsPtrType<Core>::Type GpuParamsType;

	public:
		TGpuDataParam();
		TGpuDataParam(GpuParamDataDesc* paramDesc, const GpuParamsType& parent);

		/**
		 * Sets a parameter value at the specified array index. If parameter does not contain an array leave the index at 0.
		 *
		 * @note	
		 * Like with all GPU parameters, the actual GPU buffer will not be updated until rendering with material this 
		 * parameter was created from starts on the core thread.
		 */
		void set(const T& value, UINT32 arrayIdx = 0);

		/**
		 * Returns a value of a parameter at the specified array index. If parameter does not contain an array leave the 
		 * index at 0.
		 *
		 * @note	No GPU reads are done. Data returned was cached when it was written. 
		 */
		T get(UINT32 arrayIdx = 0);

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t &nullval) const
		{
			return mParamDesc == nullptr;
		}

	protected:
		GpuParamsType mParent;
		GpuParamDataDesc* mParamDesc;
	};

	/** @copydoc TGpuDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TGpuParamStruct
	{
	public:
		typedef typename TGpuParamBufferType<Core>::Type GpuParamBufferType;
		typedef typename TGpuParamsPtrType<Core>::Type GpuParamsType;

		TGpuParamStruct();
		TGpuParamStruct(GpuParamDataDesc* paramDesc, const GpuParamsType& parent);

		/** @copydoc TGpuDataParam::set */
		void set(const void* value, UINT32 sizeBytes, UINT32 arrayIdx = 0);

		/** @copydoc TGpuDataParam::get */
		void get(void* value, UINT32 sizeBytes, UINT32 arrayIdx = 0);

		/**	Returns the size of the struct in bytes. */
		UINT32 getElementSize() const;

		/**	Checks if param is initialized. */
		bool operator==(const nullptr_t &nullval) const
		{
			return mParamDesc == nullptr;
		}

	protected:
		GpuParamsType mParent;
		GpuParamDataDesc* mParamDesc;
	};

	/** @copydoc TGpuDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TGpuParamTexture
	{
	private:
		friend class GpuParams;
		friend class GpuParamsCore;

		typedef typename TGpuParamsPtrType<Core>::Type GpuParamsType;
		typedef typename TGpuParamTextureType<Core>::Type TextureType;

	public:
		TGpuParamTexture();
		TGpuParamTexture(GpuParamObjectDesc* paramDesc, const GpuParamsType& parent);

		/** @copydoc TGpuDataParam::set */
		void set(const TextureType& texture);

		/** @copydoc TGpuDataParam::get */
		TextureType get();

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t &nullval) const
		{
			return mParamDesc == nullptr;
		}

	protected:
		GpuParamsType mParent;
		GpuParamObjectDesc* mParamDesc;
	};

	/** @copydoc TGpuDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TGpuParamLoadStoreTexture
	{
	private:
		friend class GpuParams;
		friend class GpuParamsCore;

		typedef typename TGpuParamsPtrType<Core>::Type GpuParamsType;
		typedef typename TGpuParamTextureType<Core>::Type TextureType;

	public:
		TGpuParamLoadStoreTexture();
		TGpuParamLoadStoreTexture(GpuParamObjectDesc* paramDesc, const GpuParamsType& parent);

		/** @copydoc TGpuDataParam::set */
		void set(const TextureType& texture, const TextureSurface& surface);

		/** @copydoc TGpuDataParam::get */
		TextureType get();

		/**	Checks if param is initialized. */
		bool operator==(const nullptr_t &nullval) const
		{
			return mParamDesc == nullptr;
		}

	protected:
		GpuParamsType mParent;
		GpuParamObjectDesc* mParamDesc;
	};

	/** @copydoc TGpuDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TGpuParamSampState
	{
	private:
		friend class GpuParams;
		friend class GpuParamsCore;

		typedef typename TGpuParamsPtrType<Core>::Type GpuParamsType;
		typedef typename TGpuParamSamplerStateType<Core>::Type SamplerStateType;

	public:
		TGpuParamSampState();
		TGpuParamSampState(GpuParamObjectDesc* paramDesc, const GpuParamsType& parent);

		/** @copydoc TGpuDataParam::set */
		void set(const SamplerStateType& samplerState);

		/** @copydoc TGpuDataParam::get */
		SamplerStateType get();

		/**	Checks if param is initialized. */
		bool operator==(const nullptr_t &nullval) const
		{
			return mParamDesc == nullptr;
		}

	protected:
		GpuParamsType mParent;
		GpuParamObjectDesc* mParamDesc;
	};

	/** @} */

	/** @addtogroup RenderAPI
	 *  @{
	 */

	typedef TGpuDataParam<float, false> GpuParamFloat;
	typedef TGpuDataParam<Vector2, false> GpuParamVec2;
	typedef TGpuDataParam<Vector3, false> GpuParamVec3;
	typedef TGpuDataParam<Vector4, false> GpuParamVec4;
	typedef TGpuDataParam<int, false> GpuParamInt;
	typedef TGpuDataParam<Vector2I, false> GpuParamVec2I;
	typedef TGpuDataParam<Vector3I, false> GpuParamVec3I;
	typedef TGpuDataParam<Vector4I, false> GpuParamVec4I;
	typedef TGpuDataParam<Matrix3, false> GpuParamMat3;
	typedef TGpuDataParam<Matrix4, false> GpuParamMat4;
	typedef TGpuDataParam<Color, false> GpuParamColor;

	typedef TGpuDataParam<float, true> GpuParamFloatCore;
	typedef TGpuDataParam<Vector2, true> GpuParamVec2Core;
	typedef TGpuDataParam<Vector3, true> GpuParamVec3Core;
	typedef TGpuDataParam<Vector4, true> GpuParamVec4Core;
	typedef TGpuDataParam<int, true> GpuParamIntCore;
	typedef TGpuDataParam<Vector2I, true> GpuParamVec2ICore;
	typedef TGpuDataParam<Vector3I, true> GpuParamVec3ICore;
	typedef TGpuDataParam<Vector4I, true> GpuParamVec4ICore;
	typedef TGpuDataParam<Matrix3, true> GpuParamMat3Core;
	typedef TGpuDataParam<Matrix4, true> GpuParamMat4Core;
	typedef TGpuDataParam<Color, true> GpuParamColorCore;

	typedef TGpuParamStruct<false> GpuParamStruct;
	typedef TGpuParamStruct<true> GpuParamStructCore;

	typedef TGpuParamTexture<false> GpuParamTexture;
	typedef TGpuParamTexture<true> GpuParamTextureCore;

	typedef TGpuParamSampState<false> GpuParamSampState;
	typedef TGpuParamSampState<true> GpuParamSampStateCore;

	typedef TGpuParamLoadStoreTexture<false> GpuParamLoadStoreTexture;
	typedef TGpuParamLoadStoreTexture<true> GpuParamLoadStoreTextureCore;

	/** @} */
}