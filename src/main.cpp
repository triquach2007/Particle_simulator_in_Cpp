#include <iostream>
#include <cstdlib>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "../include/particle_manager.h"
#include "../include/physics_engine.h"
#include "../include/constant.h"

using namespace std;


// ============================================================
// Globals
// ============================================================

SDL_Window* window = nullptr;
SDL_GPUDevice* device = nullptr;

Particle_manager pm;
Physics_engine pe(&pm);

SDL_GPUShader* vertex_shader = nullptr;
SDL_GPUShader* fragment_shader = nullptr;
SDL_GPUGraphicsPipeline* particle_pipeline = nullptr;

SDL_GPUBuffer* particle_buffer = nullptr;
SDL_GPUTransferBuffer* transfer_buffer = nullptr;

// ============================================================
// Physics
// ============================================================

void event_update(const double& dt)
{
    pe.update(dt);
}

// ============================================================
// Particle vertex
// ============================================================

struct ParticleVertex
{
    float x;
    float y;
};


// ============================================================
// Load SPIR-V shader
// ============================================================

SDL_GPUShader* load_shader(
    const char* filename,
    SDL_GPUShaderStage stage,
    Uint32 num_uniform_buffers)
{
    size_t code_size = 0;

    void* code = SDL_LoadFile(
        filename,
        &code_size
    );

    if (!code)
    {
        SDL_Log(
            "Could not load shader %s: %s",
            filename,
            SDL_GetError()
        );

        return nullptr;
    }

    SDL_GPUShaderCreateInfo info = {};

    info.code = static_cast<Uint8*>(code);
    info.code_size = code_size;

    info.entrypoint = "main";

    info.format = SDL_GPU_SHADERFORMAT_SPIRV;

    info.stage = stage;

    info.num_samplers = 0;
    info.num_storage_textures = 0;
    info.num_storage_buffers = 0;
    info.num_uniform_buffers = num_uniform_buffers;

    SDL_GPUShader* shader =
        SDL_CreateGPUShader(
            device,
            &info
        );

    SDL_free(code);

    if (!shader)
    {
        SDL_Log(
            "Could not create shader %s: %s",
            filename,
            SDL_GetError()
        );
    }

    return shader;
}


// ============================================================
// Create GPU particle buffer
// ============================================================

bool create_particle_buffer()
{
    const Uint32 max_particles = 100000;

    Uint32 buffer_size =
        max_particles * sizeof(ParticleVertex);


    // --------------------------------------------------------
    // GPU vertex buffer
    // --------------------------------------------------------

    SDL_GPUBufferCreateInfo buffer_info = {};

    buffer_info.usage =
        SDL_GPU_BUFFERUSAGE_VERTEX;

    buffer_info.size =
        buffer_size;


    particle_buffer =
        SDL_CreateGPUBuffer(
            device,
            &buffer_info
        );


    if (!particle_buffer)
    {
        SDL_Log(
            "Could not create particle buffer: %s",
            SDL_GetError()
        );

        return false;
    }


    // --------------------------------------------------------
    // Transfer buffer
    // --------------------------------------------------------

    SDL_GPUTransferBufferCreateInfo transfer_info = {};

    transfer_info.usage =
        SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;

    transfer_info.size =
        buffer_size;


    transfer_buffer =
        SDL_CreateGPUTransferBuffer(
            device,
            &transfer_info
        );


    if (!transfer_buffer)
    {
        SDL_Log(
            "Could not create transfer buffer: %s",
            SDL_GetError()
        );

        return false;
    }


    return true;
}


// ============================================================
// Create graphics pipeline
// ============================================================

bool create_particle_pipeline()
{
    vertex_shader =
        load_shader(
            "shaders/particle.vert.spv",
            SDL_GPU_SHADERSTAGE_VERTEX,
            1
        );

    if (!vertex_shader)
        return false;


    fragment_shader =
        load_shader(
            "shaders/particle.frag.spv",
            SDL_GPU_SHADERSTAGE_FRAGMENT,
            0
        );

    if (!fragment_shader)
        return false;


    // --------------------------------------------------------
    // Vertex buffer layout
    // --------------------------------------------------------

    SDL_GPUVertexBufferDescription vertex_buffer_desc = {};

    vertex_buffer_desc.slot = 0;

    vertex_buffer_desc.pitch =
        sizeof(ParticleVertex);

    vertex_buffer_desc.input_rate =
        SDL_GPU_VERTEXINPUTRATE_VERTEX;

    vertex_buffer_desc.instance_step_rate = 0;


    // --------------------------------------------------------
    // Position attribute
    // --------------------------------------------------------

    SDL_GPUVertexAttribute position_attribute = {};

    position_attribute.location = 0;

    position_attribute.buffer_slot = 0;

    position_attribute.format =
        SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;

    position_attribute.offset =
        0;


    SDL_GPUVertexInputState vertex_input = {};

    vertex_input.num_vertex_buffers = 1;

    vertex_input.vertex_buffer_descriptions =
        &vertex_buffer_desc;

    vertex_input.num_vertex_attributes = 1;

    vertex_input.vertex_attributes =
        &position_attribute;


    // --------------------------------------------------------
    // Color target
    // --------------------------------------------------------

    SDL_GPUColorTargetDescription color_target = {};

    color_target.format =
        SDL_GetGPUSwapchainTextureFormat(
            device,
            window
        );


    // --------------------------------------------------------
    // Pipeline
    // --------------------------------------------------------

    SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {};

    pipeline_info.vertex_shader =
        vertex_shader;

    pipeline_info.fragment_shader =
        fragment_shader;

    pipeline_info.vertex_input_state =
        vertex_input;

    pipeline_info.primitive_type =
        SDL_GPU_PRIMITIVETYPE_POINTLIST;


    pipeline_info.target_info.num_color_targets =
        1;

    pipeline_info.target_info.color_target_descriptions =
        &color_target;


    particle_pipeline =
        SDL_CreateGPUGraphicsPipeline(
            device,
            &pipeline_info
        );


    if (!particle_pipeline)
    {
        SDL_Log(
            "Could not create GPU pipeline: %s",
            SDL_GetError()
        );

        return false;
    }


    return true;
}


// ============================================================
// Upload particles
// ============================================================

bool upload_particles(
    SDL_GPUCommandBuffer* command_buffer)
{
    const vector<SDL_FPoint>& points =
        pm.get_render_buffer();


    if (points.empty())
        return true;


    // --------------------------------------------------------
    // Map transfer buffer
    // --------------------------------------------------------

    ParticleVertex* mapped =
        static_cast<ParticleVertex*>(
            SDL_MapGPUTransferBuffer(
                device,
                transfer_buffer,
                true
            )
        );


    if (!mapped)
    {
        SDL_Log(
            "Could not map transfer buffer: %s",
            SDL_GetError()
        );

        return false;
    }


    // --------------------------------------------------------
    // Copy CPU particles -> transfer buffer
    // --------------------------------------------------------

    for (size_t i = 0; i < points.size(); i++)
    {
        mapped[i].x =
            points[i].x;

        mapped[i].y =
            points[i].y;
    }


    SDL_UnmapGPUTransferBuffer(
        device,
        transfer_buffer
    );


    // --------------------------------------------------------
    // Copy transfer buffer -> GPU vertex buffer
    // --------------------------------------------------------

    SDL_GPUCopyPass* copy_pass =
        SDL_BeginGPUCopyPass(
            command_buffer
        );


    SDL_GPUTransferBufferLocation source = {};

    source.transfer_buffer =
        transfer_buffer;

    source.offset =
        0;


    SDL_GPUBufferRegion destination = {};

    destination.buffer =
        particle_buffer;

    destination.offset =
        0;

    destination.size =
        static_cast<Uint32>(
            points.size() *
            sizeof(ParticleVertex)
        );


    SDL_UploadToGPUBuffer(
        copy_pass,
        &source,
        &destination,
        true
    );


    SDL_EndGPUCopyPass(
        copy_pass
    );


    return true;
}


// ============================================================
// Render
// ============================================================

void render_update()
{
    SDL_GPUCommandBuffer* command_buffer =
        SDL_AcquireGPUCommandBuffer(
            device
        );


    if (!command_buffer)
    {
        SDL_Log(
            "SDL_AcquireGPUCommandBuffer failed: %s",
            SDL_GetError()
        );

        return;
    }


    SDL_GPUTexture* swapchain_texture =
        nullptr;

    Uint32 width = 0;
    Uint32 height = 0;


    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            command_buffer,
            window,
            &swapchain_texture,
            &width,
            &height))
    {
        SDL_Log(
            "Could not acquire swapchain: %s",
            SDL_GetError()
        );

        SDL_SubmitGPUCommandBuffer(
            command_buffer
        );

        return;
    }


    if (!swapchain_texture)
    {
        SDL_SubmitGPUCommandBuffer(
            command_buffer
        );

        return;
    }


    // --------------------------------------------------------
    // Upload particle positions
    // --------------------------------------------------------

    if (!upload_particles(command_buffer))
    {
        SDL_SubmitGPUCommandBuffer(
            command_buffer
        );

        return;
    }


    // --------------------------------------------------------
    // Render target
    // --------------------------------------------------------

    SDL_GPUColorTargetInfo color_target = {};

    color_target.texture =
        swapchain_texture;

    color_target.load_op =
        SDL_GPU_LOADOP_CLEAR;

    color_target.store_op =
        SDL_GPU_STOREOP_STORE;

    color_target.clear_color = {
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };


    SDL_GPURenderPass* render_pass =
        SDL_BeginGPURenderPass(
            command_buffer,
            &color_target,
            1,
            nullptr
        );


    // --------------------------------------------------------
    // Bind particle pipeline
    // --------------------------------------------------------

    SDL_BindGPUGraphicsPipeline(
        render_pass,
        particle_pipeline
    );


    // --------------------------------------------------------
    // Bind vertex buffer
    // --------------------------------------------------------

    SDL_GPUBufferBinding vertex_binding = {};

    vertex_binding.buffer =
        particle_buffer;

    vertex_binding.offset =
        0;


    SDL_BindGPUVertexBuffers(
        render_pass,
        0,
        &vertex_binding,
        1
    );


    // --------------------------------------------------------
    // Camera data
    // --------------------------------------------------------

    struct Camera
    {
        float width;
        float height;
    };


    Camera camera = {
        static_cast<float>(width),
        static_cast<float>(height)
    };


    SDL_PushGPUVertexUniformData(
        command_buffer,
        0,
        &camera,
        sizeof(Camera)
    );


    // --------------------------------------------------------
    // Draw particles
    // --------------------------------------------------------

    const vector<SDL_FPoint>& points =
        pm.get_render_buffer();


    if (!points.empty())
    {
        SDL_DrawGPUPrimitives(
            render_pass,
            static_cast<Uint32>(
                points.size()
            ),
            1,
            0,
            0
        );
    }


    SDL_EndGPURenderPass(
        render_pass
    );


    // --------------------------------------------------------
    // Submit
    // --------------------------------------------------------

    SDL_SubmitGPUCommandBuffer(
        command_buffer
    );
}


// ============================================================
// Main
// ============================================================

int main(int argc, char* argv[])
{
    // --------------------------------------------------------
    // SDL
    // --------------------------------------------------------

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log(
            "SDL_Init failed: %s",
            SDL_GetError()
        );

        return 1;
    }


    // --------------------------------------------------------
    // Window
    // --------------------------------------------------------

    window =
        SDL_CreateWindow(
            "Particle Simulator",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            0
        );


    if (!window)
    {
        SDL_Log(
            "SDL_CreateWindow failed: %s",
            SDL_GetError()
        );

        SDL_Quit();

        return 1;
    }


    // --------------------------------------------------------
    // GPU
    // --------------------------------------------------------

    device =
        SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV,
            true,
            nullptr
        );


    if (!device)
    {
        SDL_Log(
            "SDL_CreateGPUDevice failed: %s",
            SDL_GetError()
        );

        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }


    SDL_Log(
        "GPU driver: %s",
        SDL_GetGPUDeviceDriver(device)
    );


    // --------------------------------------------------------
    // Claim window
    // --------------------------------------------------------

    if (!SDL_ClaimWindowForGPUDevice(
            device,
            window))
    {
        SDL_Log(
            "SDL_ClaimWindowForGPUDevice failed: %s",
            SDL_GetError()
        );

        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }


    // --------------------------------------------------------
    // Create particles
    // --------------------------------------------------------

    for (int i = 0; i < 2000; i++)
    {
        Particle* p =
            new Particle;


        p->position = {
            static_cast<float>(rand()) /
                static_cast<float>(RAND_MAX)
                * 1100.0f + 50.0f,

            static_cast<float>(rand()) /
                static_cast<float>(RAND_MAX)
                * 500.0f + 50.0f
        };


        p->velocity = {
            0.0f,
            0.0f
        };


        p->acceleration = {
            0.0f,
            0.0f
        };


        pm.append_particle(p);
    }


    // --------------------------------------------------------
    // GPU resources
    // --------------------------------------------------------

    if (!create_particle_buffer())
    {
        SDL_ReleaseWindowFromGPUDevice(
            device,
            window
        );

        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }


    if (!create_particle_pipeline())
    {
        SDL_ReleaseGPUBuffer(
            device,
            particle_buffer
        );

        SDL_ReleaseGPUTransferBuffer(
            device,
            transfer_buffer
        );

        SDL_ReleaseWindowFromGPUDevice(
            device,
            window
        );

        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }


    // --------------------------------------------------------
    // Timing
    // --------------------------------------------------------

    Uint64 last_time =
        SDL_GetPerformanceCounter();

    Uint64 fps_timer =
        last_time;

    int frame_count = 0;

    bool done = false;


    // ========================================================
    // Main loop
    // ========================================================

    while (!done)
    {
        SDL_Event event;


        while (SDL_PollEvent(&event))
        {
            if (event.type ==
                SDL_EVENT_QUIT)
            {
                done = true;
            }
        }


        // ----------------------------------------------------
        // Delta time
        // ----------------------------------------------------

        Uint64 current_time =
            SDL_GetPerformanceCounter();


        double dt =
            static_cast<double>(
                current_time - last_time
            )
            /
            static_cast<double>(
                SDL_GetPerformanceFrequency()
            );


        // ----------------------------------------------------
        // Physics
        // ----------------------------------------------------

        event_update(dt);


        // ----------------------------------------------------
        // GPU rendering
        // ----------------------------------------------------

        render_update();


        // ----------------------------------------------------
        // FPS
        // ----------------------------------------------------

        frame_count++;


        double elapsed =
            static_cast<double>(
                current_time - fps_timer
            )
            /
            static_cast<double>(
                SDL_GetPerformanceFrequency()
            );


        if (elapsed >= 1.0)
        {
            double fps =
                static_cast<double>(
                    frame_count
                ) / elapsed;


            SDL_Log(
                "FPS: %.1f | Frame: %.3f ms",
                fps,
                1000.0 / fps
            );


            frame_count = 0;
            fps_timer = current_time;
        }


        last_time =
            current_time;
    }


    // ========================================================
    // Cleanup
    // ========================================================

    SDL_ReleaseGPUGraphicsPipeline(
        device,
        particle_pipeline
    );

    SDL_ReleaseGPUShader(
        device,
        vertex_shader
    );

    SDL_ReleaseGPUShader(
        device,
        fragment_shader
    );

    SDL_ReleaseGPUBuffer(
        device,
        particle_buffer
    );

    SDL_ReleaseGPUTransferBuffer(
        device,
        transfer_buffer
    );

    SDL_ReleaseWindowFromGPUDevice(
        device,
        window
    );

    SDL_DestroyGPUDevice(device);

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}