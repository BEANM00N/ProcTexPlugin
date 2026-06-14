#include "ModelPreviewWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Components/PostProcessComponent.h" 

// --- Wrap Editor-Only Includes ---
#if WITH_EDITOR
#include "AdvancedPreviewScene.h"
#include "SEditorViewport.h"
#include "EditorViewportClient.h"

// =========================================================
// 1. Custom Viewport Client to Override the Background
// =========================================================
class FMyPreviewViewportClient : public FEditorViewportClient
{
public:
    FMyPreviewViewportClient(FPreviewScene* InPreviewScene)
        : FEditorViewportClient(nullptr, InPreviewScene)
    {
    }

    // This overrides the default engine color with your specific Hex!
    virtual FLinearColor GetBackgroundColor() const override
    {
        return FColor::FromHex(TEXT("#131313"));
    }
};

// =========================================================
// 2. The Slate Viewport Implementation (Editor Only)
// =========================================================
class SMyCustomViewport : public SEditorViewport
{
public:
    SLATE_BEGIN_ARGS(SMyCustomViewport) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedPtr<FAdvancedPreviewScene> InPreviewScene)
    {
        PreviewScene = InPreviewScene;
        SEditorViewport::Construct(SEditorViewport::FArguments());
    }

    // We expose our Viewport Client here so the UMG wrapper can access the camera
    TSharedPtr<FEditorViewportClient> MyViewportClient;

protected:
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override
    {
        // Creates the camera and controls for the viewport using our custom class
        MyViewportClient = MakeShareable(new FMyPreviewViewportClient(PreviewScene.Get()));
        
        // Setup default camera settings
        MyViewportClient->bSetListenerPosition = false;
        MyViewportClient->SetRealtime(true); // Ensure it ticks for live previews
        MyViewportClient->SetViewLocation(FVector(200.f, 200.f, 200.f));
        MyViewportClient->SetViewRotation(FRotator(-45.f, -45.f, 0.f));
        
        return MyViewportClient.ToSharedRef();
    }

private:
    TSharedPtr<FAdvancedPreviewScene> PreviewScene;
};
#endif // WITH_EDITOR

// =========================================================
// 3. The UMG Wrapper Implementation
// =========================================================
UModelPreviewWidget::UModelPreviewWidget()
{
    // Do not create the scene in the constructor to avoid editor crashes during CDO initialization
}

UModelPreviewWidget::~UModelPreviewWidget()
{
// --- Wrap Editor-Only Cleanup ---
#if WITH_EDITOR
    if (PreviewScene.IsValid())
    {
        PreviewScene.Reset();
    }
#endif
}

TSharedRef<SWidget> UModelPreviewWidget::RebuildWidget()
{
// --- Wrap Editor-Only Widget Generation ---
#if WITH_EDITOR
    if (IsDesignTime())
    {
        // Return a blank box if we are just looking at the UMG designer
        return SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock).Text(FText::FromString("3D Preview Viewport"))
            ];
    }

    // Initialize the isolated 3D scene
    FPreviewScene::ConstructionValues ConstructionValues;
    ConstructionValues.bCreatePhysicsScene = false;
    PreviewScene = MakeShareable(new FAdvancedPreviewScene(ConstructionValues));
    
    // Hide the default environment and floor
    PreviewScene->SetFloorVisibility(false, true);
    PreviewScene->SetEnvironmentVisibility(false, true);

    // Add Post Processing for Exposure Control
    UPostProcessComponent* PPComp = NewObject<UPostProcessComponent>(GetTransientPackage(), NAME_None, RF_Transient);
    PPComp->bUnbound = true; // Ensure it affects the entire viewport camera
    
    // Set Min/Max Exposure Limits (Using UE5 EV100 settings)
    PPComp->Settings.bOverride_AutoExposureMinBrightness = true;
    PPComp->Settings.AutoExposureMinBrightness = -1.0;
    PPComp->Settings.bOverride_AutoExposureMaxBrightness = true;
    PPComp->Settings.AutoExposureMaxBrightness = 1.0f;
    PPComp->Settings.bOverride_AutoExposureBias = true;
    PPComp->Settings.AutoExposureBias = 0.0f; // Adjust this if it's still slightly too dark/bright
    
    PreviewScene->AddComponent(PPComp, FTransform::Identity);

    // Add static mesh component
    PreviewMeshComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
    PreviewScene->AddComponent(PreviewMeshComponent, FTransform::Identity);

    // Add skeletal mesh component
    PreviewSkeletalMeshComponent = NewObject<USkeletalMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
    PreviewScene->AddComponent(PreviewSkeletalMeshComponent, FTransform::Identity);

    // Build the Slate Viewport widget and pass it our scene
    ViewportWidget = SNew(SMyCustomViewport, PreviewScene);

    return ViewportWidget.ToSharedRef();

#else
    // Fallback for packaged games: just return an empty box so the game doesn't crash
    return SNew(SBox);
#endif
}

void UModelPreviewWidget::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    
#if WITH_EDITOR
    ViewportWidget.Reset();
    PreviewScene.Reset();
#endif
}

void UModelPreviewWidget::SetStaticMesh(UStaticMesh* NewMesh)
{
#if WITH_EDITOR
    if (PreviewMeshComponent && PreviewSkeletalMeshComponent)
    {
        // Clear the skeletal mesh so they don't overlap
        PreviewSkeletalMeshComponent->SetSkeletalMeshAsset(nullptr);

        PreviewMeshComponent->SetStaticMesh(NewMesh);

        // Auto-Focus the Camera
        if (NewMesh && ViewportWidget.IsValid() && ViewportWidget->MyViewportClient.IsValid())
        {
            ViewportWidget->MyViewportClient->FocusViewportOnBox(PreviewMeshComponent->Bounds.GetBox(), true);
        }
    }
#endif
}

void UModelPreviewWidget::SetSkeletalMesh(USkeletalMesh* NewMesh)
{
#if WITH_EDITOR
    if (PreviewMeshComponent && PreviewSkeletalMeshComponent)
    {
        // Clear the static mesh so they don't overlap
        PreviewMeshComponent->SetStaticMesh(nullptr);

        // UE5+ uses SetSkeletalMeshAsset
        PreviewSkeletalMeshComponent->SetSkeletalMeshAsset(NewMesh);

        // Auto-Focus the Camera
        if (NewMesh && ViewportWidget.IsValid() && ViewportWidget->MyViewportClient.IsValid())
        {
            ViewportWidget->MyViewportClient->FocusViewportOnBox(PreviewSkeletalMeshComponent->Bounds.GetBox(), true);
        }
    }
#endif
}

void UModelPreviewWidget::SetMaterial(int32 ElementIndex, UMaterialInterface* Material)
{
#if WITH_EDITOR
    // Apply the material unconditionally to both components.
    // This ensures the component "remembers" the material override even if 
    // the blueprint calls SetMaterial *before* a mesh is actually assigned.
    
    if (PreviewMeshComponent)
    {
        PreviewMeshComponent->SetMaterial(ElementIndex, Material);
    }
    
    if (PreviewSkeletalMeshComponent)
    {
        PreviewSkeletalMeshComponent->SetMaterial(ElementIndex, Material);
    }
#endif
}

void UModelPreviewWidget::ApplyMeshAsset()
{
#if WITH_EDITOR
    // Did the user pick a Static Mesh?
    if (UStaticMesh* SM = Cast<UStaticMesh>(PreviewMeshAsset))
    {
        SetStaticMesh(SM);
    }
    // Did the user pick a Skeletal Mesh?
    else if (USkeletalMesh* SKM = Cast<USkeletalMesh>(PreviewMeshAsset))
    {
        SetSkeletalMesh(SKM);
    }
    // Did the user clear the field?
    else 
    {
        SetStaticMesh(nullptr);
        SetSkeletalMesh(nullptr);
    }
#endif
}