#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "ModelPreviewWidget.generated.h"

// --- Wrap Editor-Only Forward Declarations ---
#if WITH_EDITOR
class FAdvancedPreviewScene;
class SEditorViewport;
#endif

class UStaticMeshComponent;
class UStaticMesh;
class USkeletalMeshComponent; // <--- NEW
class USkeletalMesh;          // <--- NEW
class UMaterialInterface; 

UCLASS()
class PROCTEXPLUGIN_API UModelPreviewWidget : public UWidget // Ensure your API macro is here!
{
    GENERATED_BODY()

public:
    UModelPreviewWidget();
    virtual ~UModelPreviewWidget();

    // --- NEW: A single property that accepts either mesh type ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Preview", meta = (AllowedClasses = "StaticMesh,SkeletalMesh"))
    UObject* PreviewMeshAsset;

    // --- NEW: A helper function to apply whatever is in the PreviewMeshAsset variable ---
    UFUNCTION(BlueprintCallable, Category = "3D Preview")
    void ApplyMeshAsset();

    // Set a Static Mesh for preview
    UFUNCTION(BlueprintCallable, Category = "3D Preview")
    void SetStaticMesh(UStaticMesh* NewMesh);

    // <--- NEW: Function for Skeletal Meshes
    UFUNCTION(BlueprintCallable, Category = "3D Preview")
    void SetSkeletalMesh(USkeletalMesh* NewMesh);

    UFUNCTION(BlueprintCallable, Category = "3D Preview")
    void SetMaterial(int32 ElementIndex, UMaterialInterface* Material);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
#if WITH_EDITOR
    TSharedPtr<class SMyCustomViewport> ViewportWidget;
    TSharedPtr<FAdvancedPreviewScene> PreviewScene;
    UStaticMeshComponent* PreviewMeshComponent;
    USkeletalMeshComponent* PreviewSkeletalMeshComponent; // <--- NEW
#endif
};