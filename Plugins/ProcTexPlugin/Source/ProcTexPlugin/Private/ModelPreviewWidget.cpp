#include "ModelPreviewWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Components/PostProcessComponent.h" 

#if WITH_EDITOR
#include "AdvancedPreviewScene.h"
#include "SEditorViewport.h"
#include "EditorViewportClient.h"

// Little bit of brute forcing here for that custom background
class FMyPreviewViewportClient : public FEditorViewportClient
{
public:
    FMyPreviewViewportClient(FPreviewScene* InPreviewScene)
        : FEditorViewportClient(nullptr, InPreviewScene)
    {
    }
    virtual FLinearColor GetBackgroundColor() const override
    {
        return FColor::FromHex(TEXT("#131313"));
    }
};

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
    
    TSharedPtr<FEditorViewportClient> MyViewportClient;

protected:
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override
    {
        // Create camera
        MyViewportClient = MakeShareable(new FMyPreviewViewportClient(PreviewScene.Get()));
        MyViewportClient->bSetListenerPosition = false;
        MyViewportClient->SetRealtime(true); // Ensure it ticks for live previews
        MyViewportClient->SetViewLocation(FVector(200.f, 200.f, 200.f));
        MyViewportClient->SetViewRotation(FRotator(-45.f, -45.f, 0.f));
        return MyViewportClient.ToSharedRef();
    }

private:
    TSharedPtr<FAdvancedPreviewScene> PreviewScene;
};
#endif

UModelPreviewWidget::UModelPreviewWidget()
{
    
}

UModelPreviewWidget::~UModelPreviewWidget()
{
#if WITH_EDITOR
    if (PreviewScene.IsValid())
    {
        PreviewScene.Reset();
    }
#endif
}

TSharedRef<SWidget> UModelPreviewWidget::RebuildWidget()
{
#if WITH_EDITOR
    if (IsDesignTime())
    {
        return SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock).Text(FText::FromString("3D Preview Viewport"))
            ];
    }

    // Mini 3D scene
    FPreviewScene::ConstructionValues ConstructionValues;
    ConstructionValues.bCreatePhysicsScene = false;
    PreviewScene = MakeShareable(new FAdvancedPreviewScene(ConstructionValues));
    PreviewScene->SetFloorVisibility(false, true);
    PreviewScene->SetEnvironmentVisibility(false, true);

    UPostProcessComponent* PPComp = NewObject<UPostProcessComponent>(GetTransientPackage(), NAME_None, RF_Transient);
    PPComp->bUnbound = true;
    PPComp->Settings.bOverride_AutoExposureMinBrightness = true;
    PPComp->Settings.AutoExposureMinBrightness = -1.0;
    PPComp->Settings.bOverride_AutoExposureMaxBrightness = true;
    PPComp->Settings.AutoExposureMaxBrightness = 1.0f;
    PPComp->Settings.bOverride_AutoExposureBias = true;
    PPComp->Settings.AutoExposureBias = 0.0f;
    
    PreviewScene->AddComponent(PPComp, FTransform::Identity);
    PreviewMeshComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
    PreviewScene->AddComponent(PreviewMeshComponent, FTransform::Identity);
    PreviewSkeletalMeshComponent = NewObject<USkeletalMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
    PreviewScene->AddComponent(PreviewSkeletalMeshComponent, FTransform::Identity);
    ViewportWidget = SNew(SMyCustomViewport, PreviewScene);
    return ViewportWidget.ToSharedRef();

#else
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
        PreviewSkeletalMeshComponent->SetSkeletalMeshAsset(nullptr);
        PreviewMeshComponent->SetStaticMesh(NewMesh);
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
        PreviewMeshComponent->SetStaticMesh(nullptr);
        PreviewSkeletalMeshComponent->SetSkeletalMeshAsset(NewMesh);
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
    if (UStaticMesh* SM = Cast<UStaticMesh>(PreviewMeshAsset))
    {
        SetStaticMesh(SM);
    }
    else if (USkeletalMesh* SKM = Cast<USkeletalMesh>(PreviewMeshAsset))
    {
        SetSkeletalMesh(SKM);
    }
    else 
    {
        SetStaticMesh(nullptr);
        SetSkeletalMesh(nullptr);
    }
#endif
}