// Copyright 2026 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"

#include "BrainCloudS2SSubsystem.generated.h"

class UBrainCloudS2S;

/**
 * Game-instance subsystem that owns a UBrainCloudS2S context and ticks it
 * automatically every frame. This is the recommended entry point for
 * Blueprint-only projects.
 *
 * Usage from Blueprints:
 *   1. Get the subsystem via "Get BrainCloud S2S Subsystem" from your GameInstance.
 *   2. Call InitializeS2S(...) to create the context.
 *   3. Use GetS2SContext() to access services (GlobalFileV3, RTTComms, etc.)
 *   4. Callbacks are driven automatically — no need to call RunCallbacks().
 */
UCLASS()
class BRAINCLOUDS2SPLUGIN_API UBrainCloudS2SSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // FTickableGameObject interface
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;
    virtual bool IsTickableInEditor() const override { return false; }

    // -----------------------------------------------------------------------
    // Public API
    // -----------------------------------------------------------------------

    /**
     * Creates and stores the S2S context. Call this once at startup.
     * Subsequent calls will re-initialize the context.
     */
    UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|Subsystem", meta = (DisplayName = "Initialize S2S"))
    void InitializeS2S(
        const FString& AppId,
        const FString& ServerName,
        const FString& ServerSecret,
        const FString& Url,
        bool bAutoAuth);

    /** Returns the owned S2S context, or nullptr if not yet initialized. */
    UFUNCTION(BlueprintCallable, Category = "BrainCloud|S2S|Subsystem", meta = (DisplayName = "Get S2S Context"))
    UBrainCloudS2S* GetS2SContext() const;

private:
    UPROPERTY()
    UBrainCloudS2S* S2SContext = nullptr;
};
