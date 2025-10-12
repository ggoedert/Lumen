//==============================================================================================================================================================================
/// \file
/// \brief     main scene
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "MainScene.h"

#include <lDebugLog.h>
#include <lCamera.h>
#include <lMesh.h>
#include <lMeshFilter.h>
#include <lTexture.h>
#include <lMaterial.h>
#include <lMeshRenderer.h>
#include <lTransform.h>

bool MainScene::Load()
{
    Lumen::DebugLog::Info("MainScene::Load");

    // setup camera
    if (auto cameraLock = mCamera.lock())
    {
        if (auto transformLock = cameraLock->Transform().lock())
        {
            transformLock->SetPosition({ 0.f, 0.f, -10.f });
        }
        cameraLock->AddComponent(
            mApplication.GetEngine(),
            Lumen::Camera::Type(),
            Lumen::Camera::Params({ 0.4509f, 0.8431f, 1.f, 1.f }));
    }

    // setup sphere
    if (auto sphereLock = mSphere.lock())
    {
        // load sphere mesh
        Lumen::Expected<Lumen::ObjectPtr> meshExp = Lumen::Assets::Import(
            "Library/lumen default resources",
            Lumen::Mesh::Type(),
            "Sphere"
        );
        if (!meshExp.HasValue())
        {
            Lumen::DebugLog::Error("Unable to load default sphere mesh resource, {}", meshExp.Error());
            return false;
        }

        // add sphere mesh filter component, with mesh
        sphereLock->AddComponent(
            mApplication.GetEngine(),
            Lumen::MeshFilter::Type(),
            Lumen::MeshFilter::Params { static_pointer_cast<Lumen::Mesh>(meshExp.Value()) });

        // load default checker gray texture
        Lumen::Expected<Lumen::ObjectPtr> textureExp = Lumen::Assets::Import(
            "Resources/lumen_builtin_extra",
            Lumen::Texture::Type(),
            "Default-Checker-Gray"
        );
        if (!textureExp.HasValue())
        {
            Lumen::DebugLog::Error("Unable to load default checker gray texture resource, {}", textureExp.Error());
            return false;
        }
        const Lumen::TexturePtr texture = static_pointer_cast<Lumen::Texture>(textureExp.Value());

        // load material material
        Lumen::Expected<Lumen::ObjectPtr> materialExp = Lumen::Assets::Import(
            "Assets",
            Lumen::Material::Type(),
            "Material"
        );
        if (!materialExp.HasValue())
        {
            Lumen::DebugLog::Error("Unable to load material resource, {}", materialExp.Error());
            return false;
        }

        // add mesh renderer component, with material
        if (auto meshRendererLock = static_pointer_cast<Lumen::MeshRenderer>(sphereLock->AddComponent(
            mApplication.GetEngine(),
            Lumen::MeshRenderer::Type(),
            Lumen::MeshRenderer::Params { static_pointer_cast<Lumen::Material>(materialExp.Value()) }).lock()))
        {
            // set texture property
            meshRendererLock->SetProperty("_MainTex", texture);
        }
    }

    return true;
}

void MainScene::Unload()
{
    Lumen::DebugLog::Info("MainScene::Unload");
    mCamera.reset();
    mSphere.reset();
}
