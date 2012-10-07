//--------------------------------------------------------------------------------------
// File: nbody.cpp
//
// C++ AMP implementation n-body gravity computation
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once

#include <amp.h>
#include <d3dx11.h>
#include<amp_short_vectors.h>

using namespace concurrency;
using namespace concurrency::graphics;

struct particle
{
	particle() restrict(amp,cpu) {}

	particle(particle& a) restrict(amp,cpu) 
	{
		pos = a.pos;
		vel = a.vel;
	}

	particle operator= (particle& a) restrict(amp,cpu) 
	{
		pos = a.pos;
		vel = a.vel;
		return *this;
	}

    float_4 pos;
    float_4 vel;
};

using namespace concurrency;

#define MAX_DEVICE_COUNT 4 

template <class T>
void Swap(T *&x, T *&y )
{
	T *temp = x;
	x = y;
	y = temp;
}

inline float vec3_len_sqr(D3DXVECTOR3 v3)
{
	return v3.x*v3.x + v3.y*v3.y + v3.z*v3.z;
}

class nbody
{
public:
	nbody(ID3D11Device* pd3dDevice) :
		_accl_view(concurrency::direct3d::create_accelerator_view(reinterpret_cast<IUnknown *>(pd3dDevice)))
	{

	}

	~nbody()
	{
        release();
	}

	// Functions called by UI framework
	void verify_SSE_implementation();

	void create_accelerators(int &num_devices);
	void create_compute_buffers(particle *data, int num_bodies);

	void cpu_single_core(particle *data_in, particle *data_out, int num_bodies);
	void cpu_ppl(particle *data_in, particle *data_out, int num_bodies);

	void amp_single_gpu(int num_bodies, bool tiled);
	void amp_multi_gpu(particle *render_data, int num_bodies);

	void release();

	void load_particles(particle* pparticles, D3DXVECTOR3 center, D3DXVECTOR4 velocity, float spread, int num_particles);


private:
	// function pointer to switch to SSE nbody implementations
	void (*_pfn_CPU_kernel)(particle *data_in, particle &data_out, int num_bodies);

	// All CPU based function
	inline float signed_uint_rand()
	{
		float ret = ( float )( ( rand() % 10000 ) - 5000 );
		return ret / 5000.0f;
	}

	static void CPU_kernel(particle *data_in, particle &data_out, int num_bodies);
	static void SSE_kernel(particle *data_in, particle &data_out, int num_bodies);
	static void SSE4_kernel(particle *data_in, particle &data_out, int num_bodies);

	// CPU-GPU functions
	static float_4 to_float_4(D3DXVECTOR4 v) restrict(amp,cpu)
	{
		return float_4(v.x, v.y, v.z, v.w);
	}

	//// GPU based functions
	static void bodybody_interaction(float_4 &acc, const float_4 pos, const float_4 A) restrict(amp);
	
	// Different implementation of NBody
	void simple_implementation(array<particle, 1>& data_in, array<particle, 1>& data_out, UINT num_bodies);
	void tiling_implementation(array<particle, 1>& data_in, array<particle, 1>& data_out, int offset, int size, int num_bodies);
	void tiling_implementation_1GPU(array<particle, 1>& data_in, array<particle, 1>& data_out, int num_bodies);

	// Data
	array<particle, 1> *_pold[MAX_DEVICE_COUNT];
	array<particle, 1> *_pnew[MAX_DEVICE_COUNT];
	accelerator			_accls[MAX_DEVICE_COUNT];
	accelerator_view	_accl_view;
	int					_ndevices;
};