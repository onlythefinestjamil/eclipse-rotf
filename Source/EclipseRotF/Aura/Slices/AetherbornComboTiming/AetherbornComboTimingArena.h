#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingTypes.h"
#include "AetherbornComboTimingArena.generated.h"

class UAetherbornComboTimingFx;
class UDirectionalLightComponent;
class UFont;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UNiagaraComponent;
class UNiagaraSystem;
class UPointLightComponent;
class UPostProcessComponent;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Moody arena from src/world.js (createWorld): an inverted dome, a grid floor, the
 * target pillar, the arena lighting and the timing ring around the hero's feet.
 *
 * The whole set is built from /Engine/BasicShapes primitives in the constructor; the
 * authored look comes from the four material slots (dome, grid, ring, halo) and the
 * two Niagara slots. Every one of those slots is optional - when a slot is null the
 * matching piece is simply skipped (and warned about once), so an un-built project
 * still runs.
 *
 * Per frame the pawn pushes the combo snapshot in through SetSnapshot(), which feeds
 * the ring/halo shader scalars; landed hits call SpawnHit(), which kicks the pillar
 * (two damped springs + emissive flash) and drives the pooled impact FX.
 */
UCLASS()
class ECLIPSEROTF_API AAetherbornComboTimingArena : public AActor
{
	GENERATED_BODY()

public:
	AAetherbornComboTimingArena();

	/** Pushes the current combo snapshot into the timing ring + halo shaders. */
	void SetSnapshot(const FAetherbornComboTimingSnapshot& InSnapshot);

	/** Landed hit: kicks the pillar and spawns the pooled impact effects at Location. */
	void SpawnHit(const FVector& Location, float Damage, bool bHeavy);

	/** Returns the arena to its rest state and clears every live effect. */
	void ResetArena();

	/** World position of the pillar's glowing core - the default FX spawn point. */
	FVector GetHitPoint() const;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Arena")
	TObjectPtr<UMaterialInterface> DomeMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Arena")
	TObjectPtr<UMaterialInterface> GridMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Arena")
	TObjectPtr<UMaterialInterface> RingMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Arena")
	TObjectPtr<UMaterialInterface> HaloMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Arena")
	TObjectPtr<UNiagaraSystem> ImpactNiagara;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Arena")
	TObjectPtr<UNiagaraSystem> DustNiagara;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Arena")
	TObjectPtr<UFont> DamageFont;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Root for the whole set. */
	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Arena")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Inverted sphere that stands in for the Slice's sky / backdrop shader. */
	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Arena")
	TObjectPtr<UStaticMeshComponent> Dome;

	/** Dark floor under the grid, tinted from the palette. */
	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Arena")
	TObjectPtr<UStaticMeshComponent> Ground;

	/** Grid overlay plane, drawn with GridMaterial. */
	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Arena")
	TObjectPtr<UStaticMeshComponent> GridPlane;

	/** Base of the pillar; every pillar part is a child of this pivot. */
	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Arena")
	TObjectPtr<USceneComponent> PillarPivot;

	/** Flat ring drawn with RingMaterial at the hero's feet. */
	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Arena")
	TObjectPtr<UStaticMeshComponent> RingMesh;

	/** Additive halo that hugs the timing ring. */
	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Arena")
	TObjectPtr<UStaticMeshComponent> HaloMesh;

	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Arena")
	TObjectPtr<UDirectionalLightComponent> KeyLight;

	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Arena")
	TObjectPtr<UDirectionalLightComponent> RimLight;

	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Arena")
	TObjectPtr<UDirectionalLightComponent> FillLight;

	/** Point light living inside the pillar's core band. */
	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Arena")
	TObjectPtr<UPointLightComponent> CoreLight;

	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Arena")
	TObjectPtr<UNiagaraComponent> DustComponent;

	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Arena")
	TObjectPtr<UPostProcessComponent> Vignette;

private:
	/** Builds the stacked-cylinder pillar under PillarPivot. */
	void BuildPillar(UStaticMesh* CylinderMesh);

	/** Creates one pillar segment and files it under its material group. */
	void AddPillarPart(UStaticMesh* Mesh, float RadiusTop, float RadiusBottom, float HeightM, float CenterZM,
	                   TArray<TObjectPtr<UStaticMeshComponent>>& Group, bool bBottomAligned = false);

	/** Creates the tinted MIDs for the pillar and the ring / halo / grid MIDs. */
	void CreateDynamicMaterials();

	/** Creates a MID from Base tinted to Color, or null when Base is unusable. */
	UMaterialInstanceDynamic* MakeTintedMID(UMaterialInterface* Base, const FLinearColor& Color, const TCHAR* Purpose);

	/** Applies uFill / uWinStart / uWinCancel / uWinEnd / uOpen / uPulse to one MID. */
	void ApplyRingScalars(UMaterialInstanceDynamic* MID, float Fill, float WinStart, float WinCancel, float WinEnd,
	                      float Open, float Pulse) const;

	/** Logs a warning at most once per key, so a null slot cannot spam the log. */
	void WarnOnce(FName Key, const FString& Message);

	/** Combo snapshot pushed in by the pawn this frame. */
	UPROPERTY()
	FAetherbornComboTimingSnapshot Snapshot;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PillarStone;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PillarTrim;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PillarCore;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> StoneMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> TrimMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CoreMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> GroundMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> GridMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> RingMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> HaloMID;

	/** Pooled damage numbers + impact Niagara, created in BeginPlay. */
	UPROPERTY()
	TObjectPtr<UAetherbornComboTimingFx> Fx;

	UPROPERTY()
	TSet<FName> WarnedKeys;

	/** Seconds elapsed since BeginPlay (drives the grid scroll and ring pulse). */
	float ElapsedTime = 0.f;

	/** Pillar swing springs, ported from world.js (rot 110/11.5, z 90/9.5). */
	float KickAngle = 0.f;
	float KickAngleVel = 0.f;
	float KickOffset = 0.f;
	float KickOffsetVel = 0.f;
	float Flash = 0.f;

	/** Cheap chain length used to scale the floating damage numbers. */
	int32 HitCombo = 0;
};
