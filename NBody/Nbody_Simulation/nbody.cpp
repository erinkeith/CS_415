//--------------------------------------------------------------------------------------
// File: nbody.cpp
//
// C++ AMP implementation of n-body gravity computation
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include <string.h>
#include <math.h>
#include <ppl.h>
#include <concrtrm.h>
#include <d3dx11.h>
#include <amprt.h>
#include <amp_math.h>
#include <assert.h>

#include "dxtypes.h"
#include "nbody.h"

using namespace Concurrency;

#define TILE_SIZE                            256
#define softeningSquared                    0.0000015625f
#define fG                                    (6.67300e-11f*10000.0f)
#define fParticleMass                        (fG*10000.0f*10000.0f)
#define deltaTime                            0.1f 
#define dampening                            1.0f

extern ID3D11Buffer*                        g_pParticlePosVelo0;
extern ID3D11Buffer*                        g_pParticlePosVelo1;
extern ID3D11ShaderResourceView*            g_pParticlePosVeloRV0;
extern ID3D11ShaderResourceView*            g_pParticlePosVeloRV1;
extern ID3D11UnorderedAccessView*            g_pParticlePosVeloUAV0;
extern ID3D11UnorderedAccessView*            g_pParticlePosVeloUAV1;

// GPU based functions
void nbody::bodybody_interaction(float_4 &acc, const float_4 pos, float_4 A) restrict(amp)
{
    float_4 r = A - pos;
    
    float distSqr = r.x*r.x + r.y*r.y + r.z*r.z;
    distSqr += softeningSquared;
    
    float invDist = 1.0f / fast_math::sqrt(distSqr);
    float invDistCube =  invDist*invDist*invDist;
    
    float s = fParticleMass*invDistCube;
    
    acc += r*s;
}

// This helper function loads a group of particles
void nbody::load_particles(particle* pparticles, D3DXVECTOR3 center, D3DXVECTOR4 velocity, float spread, int num_particles)
{
    for( int i = 0; i < num_particles; i++ )
    {
        D3DXVECTOR3 delta( spread, spread, spread );

        while( vec3_len_sqr(delta) > spread * spread )
        {
            delta.x = signed_uint_rand() * spread;
            delta.y = signed_uint_rand() * spread;
            delta.z = signed_uint_rand() * spread;
        }

        pparticles[i].pos.x = center.x + delta.x;
        pparticles[i].pos.y = center.y + delta.y;
        pparticles[i].pos.z = center.z + delta.z;
        pparticles[i].pos.w = 10000.0f * 10000.0f;

        pparticles[i].vel = to_float_4(velocity);
    }    
}

void nbody::CPU_kernel(particle *data_in, particle &data_out, int num_bodies)
{
    D3DXVECTOR4 pos;
    pos.x = data_out.pos.x;
    pos.y = data_out.pos.y;
    pos.z = data_out.pos.z;
    pos.w = data_out.pos.w;

    D3DXVECTOR4 vel;
    vel.x = data_out.vel.x;
    vel.y = data_out.vel.y;
    vel.z = data_out.vel.z;
    vel.w = data_out.vel.w;
    
    D3DXVECTOR4 acc;
    acc.x = 0;
    acc.y = 0;
    acc.z = 0;
    acc.w = 0;

    for (int j = 0; j < num_bodies; j++)
    {  
        D3DXVECTOR4 r;

        r.x = data_in[j].pos.x - pos.x;
        r.y = data_in[j].pos.y - pos.y;
        r.z = data_in[j].pos.z - pos.z;

        float dist_sqr = r.x*r.x + r.y*r.y + r.z*r.z;
        dist_sqr += softeningSquared;

        float inv_dist = 1.0f / sqrt(dist_sqr);
        float inv_dist_cube =  inv_dist * inv_dist * inv_dist;
        
        float s = fParticleMass * inv_dist_cube;

        acc.x += r.x * s;
        acc.y += r.y * s;
        acc.z += r.z * s;
    }
    
    vel.x += acc.x*deltaTime;
    vel.y += acc.y*deltaTime;
    vel.z += acc.z*deltaTime;

    vel.x *= dampening;
    vel.y *= dampening;
    vel.z *= dampening;

    pos.x += vel.x*deltaTime;
    pos.y += vel.y*deltaTime;
    pos.z += vel.z*deltaTime;

    data_out.pos.x = pos.x;
    data_out.pos.y = pos.y;
    data_out.pos.z = pos.z;
    data_out.pos.w = pos.w;

    data_out.vel.x = vel.x;
    data_out.vel.y = vel.y;
    data_out.vel.z = vel.z;
    data_out.vel.w = vel.w;
}

void nbody::SSE_kernel(particle *data_in, particle &data_out, int num_bodies)
{
    const float softening_squared0 = softeningSquared;    
    const float paramf[2] = { deltaTime, 1.0f };
    const float OOOZ[4] = { 1.0f, 1.0f, 1.f, 0.0f };
    
    __m128 SSEOOOZ = _mm_loadu_ps(OOOZ);

    {
        //D3DXVECTOR3& pos = *(D3DXVECTOR3*)&B[i].pos;
        //D3DXVECTOR3& vel = *(D3DXVECTOR3*)&B[i].vel;
        //D3DXVECTOR3 acc(0,0,0);

        __m128 SSEpos = _mm_loadu_ps((float*)&data_out.pos);
        __m128 SSEvel = _mm_loadu_ps((float*)&data_out.vel);
        __m128 SSEacc = _mm_sub_ps(SSEpos, SSEpos);
        float myMass = fParticleMass;

        for (int j = 0; j < num_bodies; j++)
        {
            //D3DXVECTOR3& pos1 = *(D3DXVECTOR3*)&A[j].pos;
            __m128 SSEpos1 = _mm_loadu_ps((float*)&data_in[j].pos);

            {
                //D3DXVECTOR3 r = pos1 - pos;
                __m128 SSEr = _mm_sub_ps(SSEpos1, SSEpos);

                //float distSqr = r.x * r.x + r.y * r.y + r.z * r.z;
                // dot product
                __m128 SSEr2 = _mm_mul_ps(SSEr, SSEr);    //x    y    z    ?
                __m128 SSErshuf = _mm_shuffle_ps(SSEr2, SSEr2, _MM_SHUFFLE(0,3,2,1));
                SSEr2 = _mm_add_ps(SSEr2, SSErshuf);  //x+y, y+z, z+?, ?+x
                SSErshuf = _mm_shuffle_ps(SSEr2, SSEr2, _MM_SHUFFLE(1,0,3,2));
                SSEr2 = _mm_add_ps(SSErshuf, SSEr2);  //x+y+z+0, y+z+0+X, z+0+x+y, 0+x+y+z

                //distSqr += softeningSquared;
                __m128 SSEsoft = _mm_load1_ps( &softening_squared0 );
                SSEr2 = _mm_add_ps(SSEr2, SSEsoft); 

                //float inv_dist = 1.0f / sqrt(distSqr);
                //float inv_distCube =  inv_dist * inv_dist * inv_dist;
                __m128 SSEinvDistSqr = _mm_rsqrt_ps(SSEr2);
                __m128 SSEinvDistCube = _mm_mul_ps(_mm_mul_ps(SSEinvDistSqr,SSEinvDistSqr), SSEinvDistSqr);
                
                //float s = A[j].pos.w * invDistCube;
                //__m128 SSEmass = _mm_shuffle_ps(SSEpos1, SSEpos1, _MM_SHUFFLE(3,3,3,3));
                __m128 SSEmass = _mm_load1_ps( &myMass );
                __m128 SSEs = _mm_mul_ps(SSEmass, SSEinvDistCube); 

                //acc += r * s;
                SSEacc = _mm_add_ps( _mm_mul_ps(SSEr, SSEs), SSEacc ); 
            }
        }

        //vel += acc * paramf[0];      //deltaTime;
        __m128 SSEpf0 = _mm_load1_ps( &paramf[0] );
        SSEvel = _mm_add_ps( _mm_mul_ps(SSEacc, SSEpf0), SSEvel ); 
        //vel *= paramf[1];            //damping;
        //pos += vel * paramf[0];      //deltaTime;    
        SSEpos = _mm_add_ps( _mm_mul_ps(SSEvel, SSEpf0), SSEpos ); 
        SSEpos = _mm_mul_ps(SSEOOOZ, SSEpos); 
        
        _mm_storeu_ps((float*)&data_out.pos, SSEpos);
        _mm_storeu_ps((float*)&data_out.vel, SSEvel);
    }
}

void nbody::SSE4_kernel(particle *data_in, particle &data_out, int num_bodies)
{
    const float softening_squared0 = softeningSquared;
    const float paramf[2] = { deltaTime, 1.0f };

    {
        //D3DXVECTOR3& pos = *(D3DXVECTOR3*)&B[i].pos;
        //D3DXVECTOR3& vel = *(D3DXVECTOR3*)&B[i].vel;
        //D3DXVECTOR3 acc(0,0,0);

        //pos = *(D3DXVECTOR3*)&A[i].pos;
        //vel = *(D3DXVECTOR3*)&A[i].vel;
        __m128 SSEpos = _mm_loadu_ps((float*)&data_out.pos);
        __m128 SSEvel = _mm_loadu_ps((float*)&data_out.vel);
        __m128 SSEacc = _mm_sub_ps(SSEpos, SSEpos);
        float myMass = fParticleMass;//B.pos.w;

        for (int j = 0; j < num_bodies; j++)
        {
            //D3DXVECTOR3& pos1 = *(D3DXVECTOR3*)&A[j].pos;
            __m128 SSEpos1 = _mm_loadu_ps((float*)&data_in[j].pos);

            {
                //D3DXVECTOR3 r = pos1 - pos;
                __m128 SSEr = _mm_sub_ps(SSEpos1, SSEpos);

                //float distSqr = r.x * r.x + r.y * r.y + r.z * r.z;
                __m128 SSEr2 = _mm_mul_ps(SSEr, SSEr);    //x    y    z    ?
                __m128 SSErshuf = _mm_shuffle_ps(SSEr2, SSEr2, _MM_SHUFFLE(0,3,2,1));
                SSEr2 = _mm_add_ps(SSEr2, SSErshuf);  //x+y, y+z, z+?, ?+x
                SSErshuf = _mm_shuffle_ps(SSEr2, SSEr2, _MM_SHUFFLE(1,0,3,2));
                SSEr2 = _mm_add_ps(SSErshuf, SSEr2);  //x+y+z+0, y+z+0+X, z+0+x+y, 0+x+y+z

                //distSqr += softeningSquared;
                __m128 SSEsoft = _mm_load1_ps( &softening_squared0 );
                SSEr2 = _mm_add_ps(SSEr2, SSEsoft); 

                //float invDist = 1.0f / sqrt(distSqr);
                //float invDistCube =  invDist * invDist * invDist;
                __m128 SSEinvDistSqr = _mm_rsqrt_ps(SSEr2);
                __m128 SSEinvDistCube = _mm_mul_ps(_mm_mul_ps(SSEinvDistSqr,SSEinvDistSqr), SSEinvDistSqr);
                
                //float s = A[j].pos.w * invDistCube;
                //__m128 SSEmass = _mm_shuffle_ps(SSEpos1, SSEpos1, _MM_SHUFFLE(3,3,3,3));
                __m128 SSEmass = _mm_load1_ps( &myMass );
                __m128 SSEs = _mm_mul_ps(SSEmass, SSEinvDistCube); 

                //acc += r * s;
                SSEacc = _mm_add_ps( _mm_mul_ps(SSEr, SSEs), SSEacc ); 
            }
        }

        //vel += acc * paramf[0];      //deltaTime;
        __m128 SSEpf0 = _mm_load1_ps( &paramf[0] );
        SSEvel = _mm_add_ps( _mm_mul_ps(SSEacc, SSEpf0), SSEvel ); 
        //vel *= paramf[1];            //damping;
        //pos += vel * paramf[0];      //deltaTime;    
        SSEpos = _mm_add_ps( _mm_mul_ps(SSEvel, SSEpf0), SSEpos ); 
        
        _mm_storeu_ps((float*)&data_out.pos, SSEpos);
        _mm_storeu_ps((float*)&data_out.vel, SSEvel);
    }
}

void nbody::cpu_single_core(particle *data_in, particle *data_out, int num_bodies)
{
    for (int i = 0; i < num_bodies; i++)
    {
        data_out[i].pos = data_in[i].pos;
        data_out[i].vel = data_in[i].vel;
        
        _pfn_CPU_kernel(data_in, data_out[i], num_bodies);
    }
}

void nbody::cpu_ppl(particle *data_in, particle *data_out, int num_bodies)
{
    const int num_tasks = ::GetProcessorCount();
    parallel_for((int)0, num_tasks, [&](int task_num)
    {    
        int base_index = task_num * (num_bodies/num_tasks);
        for (int k = 0; k < (num_bodies/num_tasks); ++k)
        {
            int i = base_index + k;
            data_out[i].pos = data_in[i].pos;
            data_out[i].vel = data_in[i].vel;
            
            _pfn_CPU_kernel(data_in, data_out[i], num_bodies);
        }
    });
}

void nbody::simple_implementation(array<particle, 1>& data_in, array<particle, 1>& data_out, UINT num_bodies)
{
    extent<1> compute_domain(num_bodies);

    parallel_for_each(compute_domain, [&, num_bodies] (index<1> idx) restrict(amp)
        {
            particle p;

            p.pos = data_in[idx].pos;
            p.vel = data_in[idx].vel;
            float_4 acc(0, 0, 0, 0);

            // Update current particle using all other particles
            for (UINT j = 0; j < num_bodies; j++) 
            {
                bodybody_interaction(acc, p.pos, data_in[j].pos);
            }

            p.vel += acc*deltaTime;
            p.vel *= dampening;

            p.pos += p.vel*deltaTime;

            data_out[idx] = p;
    });
}

void nbody::tiling_implementation(array<particle, 1>& data_in, array<particle, 1>& data_out, int offset, int size, int num_bodies)
{
    static_assert(TILE_SIZE%4 == 0, "Tile size should be multiple of 4");

    extent<1> compute_domain((size/TILE_SIZE)*TILE_SIZE);

    parallel_for_each(compute_domain.tile<TILE_SIZE>(), [&, num_bodies, offset] (tiled_index<TILE_SIZE> ti) restrict(amp)
        {
            tile_static D3DXVECTOR4 tile_mem[TILE_SIZE];
    
            int idx_local = ti.local[0];
            int idx_global = ti.global[0];
    
            idx_global += offset;

            particle p;
            p.pos = data_in[idx_global].pos;
            p.vel = data_in[idx_global].vel;
            float_4 acc(0, 0, 0, 0);
    
            // Update current particle using all other particles
            int idx_temp = idx_local;
            for (int tile = 0; tile < num_bodies/TILE_SIZE; tile++)
            {
                // Cache a tile of particles into shared memory to increase IO efficiency
                tile_mem[idx_local].x = data_in[idx_temp].pos.x;
                tile_mem[idx_local].y = data_in[idx_temp].pos.y;
                tile_mem[idx_local].z = data_in[idx_temp].pos.z;
                tile_mem[idx_local].w = data_in[idx_temp].pos.w;

                ti.barrier.wait();

                // Unroll size should be multile of TILE_SIZE
                // 4 is the sweet spot - increasing further adds no perf improvement while decreasing reduces perf
                for (UINT j = 0; j < TILE_SIZE; j+=4 )
                {
                    bodybody_interaction(acc, p.pos, to_float_4(tile_mem[j+0]));
                    bodybody_interaction(acc, p.pos, to_float_4(tile_mem[j+1]));
                    bodybody_interaction(acc, p.pos, to_float_4(tile_mem[j+2]));
                    bodybody_interaction(acc, p.pos, to_float_4(tile_mem[j+3]));
                }
                ti.barrier.wait();
            }

            p.vel += acc*deltaTime;
            p.vel *= dampening;

            p.pos += p.vel*deltaTime;

            data_out[idx_global] = p;
        });
}

void nbody::tiling_implementation_1GPU(array<particle, 1>& data_in, array<particle, 1>& data_out, int size)
{
    static_assert(TILE_SIZE%4 == 0, "Tile size should be multiple of 4");

    int num_tiles = size / TILE_SIZE;
    extent<1> compute_domain(num_tiles * TILE_SIZE);

    parallel_for_each(compute_domain.tile<TILE_SIZE>(), [&, size, num_tiles] (tiled_index<TILE_SIZE> ti) restrict(amp)
        {
            tile_static D3DXVECTOR4 tile_mem[TILE_SIZE];

            int idx_local = ti.local[0];
            int idx_global = ti.global[0];

            particle p(data_in[idx_global]);
            float_4 acc(0, 0, 0, 0);
    
            // Update current particle using all other particles
            int idx_temp = idx_local;
            for (int tile = 0; tile < num_tiles; tile++, idx_temp += TILE_SIZE)
            {
                // Cache a tile of particles into shared memory to increase IO efficiency
                tile_mem[idx_local].x = data_in[idx_temp].pos.x;
                tile_mem[idx_local].y = data_in[idx_temp].pos.y;
                tile_mem[idx_local].z = data_in[idx_temp].pos.z;
                tile_mem[idx_local].w = data_in[idx_temp].pos.w;

                ti.barrier.wait();
        
                // Unroll size should be multile of TILE_SIZE
                // 4 is the sweet spot - increasing further adds no perf improvement while decreasing reduces perf
                for (int j = 0; j < TILE_SIZE; j+=4 )
                {
                    bodybody_interaction(acc, p.pos, to_float_4(tile_mem[j+0])); 
                    bodybody_interaction(acc, p.pos, to_float_4(tile_mem[j+1]));
                    bodybody_interaction(acc, p.pos, to_float_4(tile_mem[j+2]));
                    bodybody_interaction(acc, p.pos, to_float_4(tile_mem[j+3]));
                }
                ti.barrier.wait();
            }

            p.vel += acc*deltaTime;
            p.vel *= dampening;

            p.pos += p.vel*deltaTime;

            data_out[idx_global] = p;
        });
}

void nbody::verify_SSE_implementation()
{
    int CPUInfo[4] = {-1};
    __cpuid(CPUInfo, 1);
    bool bSSE1 = CPUInfo[3] >> 24 && 0x1;
    bool bSSE4 = CPUInfo[2] >> 19 && 0x1;

    if (bSSE4)
    {
        _pfn_CPU_kernel = &nbody::SSE4_kernel;
    }
    else if (bSSE1)
    {
        _pfn_CPU_kernel = &nbody::SSE_kernel;
    }
    else
    {
        _pfn_CPU_kernel = &nbody::CPU_kernel;
    }
}

void nbody::create_compute_buffers(particle *data, int num_bodies)
{
    _pold[0] = new array<particle, 1>(num_bodies, data, _accl_view);
    _pnew[0] = new array<particle, 1>(num_bodies, _accl_view);

    HRESULT hr = S_OK;
    hr = (concurrency::direct3d::get_buffer( *_pold[0] ))->QueryInterface(__uuidof(ID3D11Buffer), (LPVOID*)&g_pParticlePosVelo0);
    assert(hr == S_OK);
    hr = (concurrency::direct3d::get_buffer( *_pnew[0] ))->QueryInterface(__uuidof(ID3D11Buffer), (LPVOID*)&g_pParticlePosVelo1);
    assert(hr == S_OK);

    for(int i = 1; i < _ndevices; i++)
    {
        // create fields of size num_bodies for position and velocity vectors
        _pold[i] = new array<particle, 1>(num_bodies, data, _accls[i].default_view);
        _pnew[i] = new array<particle, 1>(num_bodies, _accls[i].default_view);
    }
}

// AMP nbody functions for DirectX sample integration
void nbody::create_accelerators(int &num_devices)
{
    std::vector<accelerator> devices = accelerator::get_all();
    _ndevices = devices.size();

    if (_ndevices == 0)
    {
        accelerator dRef = accelerator(accelerator::direct3d_ref);
        num_devices = _ndevices = 1;
        _accls[0] = dRef;
    }
    else
    {
        num_devices = 0;
        for (int i = 0; i < _ndevices; i++)
        {
            if ((!devices[i].is_emulated) && (devices[i].device_path != accelerator::cpu_accelerator))
            {
                _accls[num_devices] = devices[i];
                num_devices++;
            }
        }
        _ndevices = num_devices;
    }
}

void nbody::amp_single_gpu(int num_bodies, bool tiled)
{
    int dev_index = 0;

    if (tiled)
    {
        tiling_implementation_1GPU((*_pold[dev_index]), (*_pnew[dev_index]), num_bodies);
    }
    else
    {
        simple_implementation((*_pold[dev_index]), (*_pnew[dev_index]), num_bodies);
    }

    Swap(_pold[dev_index], _pnew[dev_index]);
    Swap( g_pParticlePosVelo0, g_pParticlePosVelo1 );
    Swap( g_pParticlePosVeloRV0, g_pParticlePosVeloRV1 );
    Swap( g_pParticlePosVeloUAV0, g_pParticlePosVeloUAV1 );
}

void nbody::amp_multi_gpu(particle *render_data, int num_bodies)
{
    int size = (int)(((num_bodies/TILE_SIZE)/_ndevices)*TILE_SIZE);

    for (int i = 0; i < _ndevices; i++)
    {
        tiling_implementation((*_pold[i]), (*_pnew[i]), i*size, size, num_bodies);
    }

    for (int i = 0; i < _ndevices; i++)
    {
        index<1> begin(i*size); 
        extent<1> end(size); 
        array_view<particle, 1> wrSrc = (*_pnew[i]).section(begin, end);

        for (int j = 0; j < size; j++)
        {
            render_data[j+(i*size)] = (wrSrc.data())[j];
        }
    }

    for (int i = 0; i < _ndevices; i++)
    {
        copy(render_data, (*_pold[i]));
    }
}

void nbody::release()
{
    for(int i = 0; i<_ndevices; i++)
    {
        if (_pold[i])
        {
            delete _pold[i];
            _pold[i] = NULL;
        }

        if (_pnew[i])
        {
            delete _pnew[i];
            _pnew[i] = NULL;
        }
    }

    if (g_pParticlePosVelo0)
        g_pParticlePosVelo0->Release();
    if (g_pParticlePosVelo1)
        g_pParticlePosVelo1->Release();
}
