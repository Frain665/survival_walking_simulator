cmake_minimum_required(VERSION 3.20)

# ============================================================
#  Проект
# ============================================================
project(SurvivalWalkingSimulator
    VERSION     1.0.0
    DESCRIPTION "Voxel survival game"
    LANGUAGES   CXX C
)

# ============================================================
#  Стандарт C++
# ============================================================
set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF) 

# ============================================================
#  Зависимости (все через vcpkg)
# ============================================================
find_package(glad       CONFIG REQUIRED)
find_package(glfw3      CONFIG REQUIRED)
find_package(glm        CONFIG REQUIRED)
find_package(imgui      CONFIG REQUIRED)
find_package(Stb 		REQUIRED MODULE)       
find_package(assimp     CONFIG REQUIRED)
find_package(OpenGL             REQUIRED)

add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/miniaudio"
)

# ============================================================
#  Исходники  (только .cpp — .h не нужны в SOURCES)
# ============================================================
set(SOURCES
    src/main.cpp

    # audio
    src/audio/AudioEngine.cpp
    src/audio/SoundBuffer.cpp
    src/audio/SoundSource.cpp

    # core
    src/core/Application.cpp
    src/core/Input.cpp
    src/core/Random.cpp
    src/core/Time.cpp
    src/core/Window.cpp

    # entities
    src/entities/Animal.cpp
    src/entities/Entity.cpp
    src/entities/EntityManager.cpp
    src/entities/Player.cpp
    src/entities/Zombie.cpp

    # entities/AI
    src/entities/AI/BehaviourTree.cpp
    src/entities/AI/Pathfinding.cpp
    src/entities/AI/StateMachine.cpp

    # inventory
    src/inventory/CraftingSystem.cpp
    src/inventory/Inventory.cpp
    src/inventory/Item.cpp
    src/inventory/ItemDatabase.cpp

    # physics
    src/physics/Collider.cpp
    src/physics/GravitySystem.cpp
    src/physics/PhysicsWorld.cpp
    src/physics/Raycast.cpp
    src/physics/RigidBody.cpp

    # renderer
    src/renderer/Camera.cpp
    src/renderer/FrustumCulling.cpp
    src/renderer/Light.cpp
    src/renderer/Material.cpp
    src/renderer/Mesh.cpp
    src/renderer/Model.cpp
    src/renderer/ParticleSystem.cpp
    src/renderer/Renderer.cpp
    src/renderer/Shader.cpp
    src/renderer/Skybox.cpp
    src/renderer/Texture.cpp

    # ui
    src/ui/HUD.cpp
    src/ui/InventoryUI.cpp
    src/ui/MainMenu.cpp
    src/ui/UIManager.cpp

    # utils
    src/utils/Logger.cpp
    src/utils/ObjectPool.cpp
    src/utils/ResourceManager.cpp

    # world
    src/world/Biome.cpp
    src/world/Chunk.cpp
    src/world/StructureGenerator.cpp
    src/world/TerrainGenerator.cpp
    src/world/TreeGenerator.cpp
    src/world/World.cpp
)

# ============================================================
#  Исполняемый файл
# ============================================================
add_executable(${PROJECT_NAME} ${SOURCES})

# ============================================================
#  Include-пути
# ============================================================
target_include_directories(${PROJECT_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"       
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty"
		${Stb_INCLUDE_DIR}
)

target_include_directories(${PROJECT_NAME} 
	PRIVATE ${Stb_INCLUDE_DIR}
)

# ============================================================
#  Линковка
# ============================================================
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        OpenGL::GL
        glfw
        glad::glad
        glm::glm
        imgui::imgui
        assimp::assimp
        miniaudio
)

# ============================================================
#  Предупреждения компилятора
# ============================================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4         
        /WX         
        /permissive-  
        /utf-8      
        /wd4100     
        /wd4505     
    )
else()
    # GCC / Clang
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wno-unused-parameter  
    )
endif()

# ============================================================
#  Оптимизация по конфигурации сборки
# ============================================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/O2>   
        $<$<CONFIG:Release>:/GL>   
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Debug>:/Zi>     
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/LTCG>
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-O3>  
        $<$<CONFIG:Release>:-march=native>
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-g3>
    )
endif()

# ============================================================
#  Определения препроцессора
# ============================================================
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
        GLFW_INCLUDE_NONE 
)

# ============================================================
#  Копирование ассетов в директорию сборки
# ============================================================
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying assets..."
)

# ============================================================
#  Вывод информации при конфигурации
# ============================================================
message(STATUS "")
message(STATUS "=== ${PROJECT_NAME} v${PROJECT_VERSION} ===")
message(STATUS "  Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ std    : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler   : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")