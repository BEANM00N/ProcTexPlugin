#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "ModelPreviewWidget.generated.h"

#if WITH_EDITOR
class FAdvancedPreviewScene;
class SEditorViewport;
#endif

class UStaticMeshComponent;
class UStaticMesh;
class USkeletalMeshComponent;
class USkeletalMesh;
class UMaterialInterface; 

UCLASS()
class PROCTEXPLUGIN_API UModelPreviewWidget : public UWidget
{
    GENERATED_BODY()

public:
    UModelPreviewWidget();
    virtual ~UModelPreviewWidget();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Preview", meta = (AllowedClasses = "StaticMesh,SkeletalMesh"))
    UObject* PreviewMeshAsset;
    
    UFUNCTION(BlueprintCallable, Category = "3D Preview")
    void ApplyMeshAsset();
    
    UFUNCTION(BlueprintCallable, Category = "3D Preview")
    void SetStaticMesh(UStaticMesh* NewMesh);
    
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
    USkeletalMeshComponent* PreviewSkeletalMeshComponent;
#endif
};