#include "EclipseKeybindSubsystem.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogEclipseKeybinds, Log, All);

namespace
{
    const TCHAR* KeybindContextPath = TEXT("/Game/Input/IMC_Default.IMC_Default");

    const TCHAR* ActionAssetPath(FName Action)
    {
        if (Action == TEXT("IA_Move")) return TEXT("/Game/Input/IA_Move.IA_Move");
        if (Action == TEXT("IA_Look")) return TEXT("/Game/Input/IA_Look.IA_Look");
        if (Action == TEXT("IA_Sprint")) return TEXT("/Game/Input/IA_Sprint.IA_Sprint");
        if (Action == TEXT("IA_Dodge")) return TEXT("/Game/Input/IA_Dodge.IA_Dodge");
        if (Action == TEXT("IA_LightAttack")) return TEXT("/Game/Input/IA_LightAttack.IA_LightAttack");
        if (Action == TEXT("IA_HeavyAttack")) return TEXT("/Game/Input/IA_HeavyAttack.IA_HeavyAttack");
        if (Action == TEXT("IA_Block")) return TEXT("/Game/Input/IA_Block.IA_Block");
        if (Action == TEXT("IA_LockOn")) return TEXT("/Game/Input/IA_LockOn.IA_LockOn");
        if (Action == TEXT("IA_Ability1")) return TEXT("/Game/Input/IA_Ability1.IA_Ability1");
        if (Action == TEXT("IA_Ability2")) return TEXT("/Game/Input/IA_Ability2.IA_Ability2");
        if (Action == TEXT("IA_Ability3")) return TEXT("/Game/Input/IA_Ability3.IA_Ability3");
        if (Action == TEXT("IA_Ability4")) return TEXT("/Game/Input/IA_Ability4.IA_Ability4");
        if (Action == TEXT("IA_Ultimate")) return TEXT("/Game/Input/IA_Ultimate.IA_Ultimate");
        if (Action == TEXT("IA_WeaponArt")) return TEXT("/Game/Input/IA_WeaponArt.IA_WeaponArt");
        if (Action == TEXT("IA_ToggleKeybinds")) return TEXT("/Game/Input/IA_ToggleKeybinds.IA_ToggleKeybinds");
        return TEXT("");
    }
}

void UEclipseKeybindSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    BuildDefaults();
    Bindings = Defaults;
    LoadBindings();
    ApplyToMappingContext();
    UE_LOG(LogEclipseKeybinds, Log, TEXT("[KEYBIND] Subsystem initialised with %d actions"), Bindings.Num());
}

void UEclipseKeybindSubsystem::BuildDefaults()
{
    Defaults.Reset();
    Defaults.Add(TEXT("IA_Move"), EKeys::W);
    Defaults.Add(TEXT("IA_Look"), EKeys::Mouse2D);
    Defaults.Add(TEXT("IA_Sprint"), EKeys::LeftShift);
    Defaults.Add(TEXT("IA_Dodge"), EKeys::SpaceBar);
    Defaults.Add(TEXT("IA_LightAttack"), EKeys::LeftMouseButton);
    Defaults.Add(TEXT("IA_HeavyAttack"), EKeys::RightMouseButton);
    Defaults.Add(TEXT("IA_Block"), EKeys::RightMouseButton);
    Defaults.Add(TEXT("IA_LockOn"), EKeys::MiddleMouseButton);
    Defaults.Add(TEXT("IA_Ability1"), EKeys::Q);
    Defaults.Add(TEXT("IA_Ability2"), EKeys::E);
    Defaults.Add(TEXT("IA_Ability3"), EKeys::One);
    Defaults.Add(TEXT("IA_Ability4"), EKeys::Two);
    Defaults.Add(TEXT("IA_Ultimate"), EKeys::R);
    Defaults.Add(TEXT("IA_WeaponArt"), EKeys::F);
    Defaults.Add(TEXT("IA_ToggleKeybinds"), EKeys::Escape);
}

FKey UEclipseKeybindSubsystem::GetBinding(FName Action) const
{
    if (const FKey* Found = Bindings.Find(Action))
    {
        return *Found;
    }
    return EKeys::Invalid;
}

FString UEclipseKeybindSubsystem::GetBindingDisplayName(FName Action) const
{
    const FKey Key = GetBinding(Action);
    return Key.IsValid() ? Key.GetDisplayName().ToString() : TEXT("Unbound");
}

FName UEclipseKeybindSubsystem::GetActionForKey(FKey Key) const
{
    for (const TPair<FName, FKey>& Pair : Bindings)
    {
        if (Pair.Value == Key)
        {
            return Pair.Key;
        }
    }
    return NAME_None;
}

bool UEclipseKeybindSubsystem::Rebind(FName Action, FKey NewKey)
{
    if (!Bindings.Contains(Action))
    {
        UE_LOG(LogEclipseKeybinds, Warning, TEXT("[KEYBIND] Rebind failed - unknown action %s"), *Action.ToString());
        return false;
    }
    if (!NewKey.IsValid())
    {
        return false;
    }
    if (NewKey == EKeys::Escape || NewKey == EKeys::LeftMouseButton || NewKey == EKeys::RightMouseButton)
    {
        UE_LOG(LogEclipseKeybinds, Warning, TEXT("[KEYBIND] Rebind rejected - %s is reserved"), *NewKey.GetDisplayName().ToString());
        return false;
    }

    const FName Existing = GetActionForKey(NewKey);
    if (!Existing.IsNone() && Existing != Action)
    {
        UE_LOG(LogEclipseKeybinds, Warning, TEXT("[KEYBIND] Conflict: %s already bound to %s"), *NewKey.GetDisplayName().ToString(), *Existing.ToString());
        return false;
    }

    Bindings[Action] = NewKey;
    ApplyToMappingContext();
    SaveBindings();
    UE_LOG(LogEclipseKeybinds, Log, TEXT("[KEYBIND] %s -> %s"), *Action.ToString(), *NewKey.GetDisplayName().ToString());
    return true;
}

void UEclipseKeybindSubsystem::ApplyToMappingContext()
{
    UInputMappingContext* Context = LoadObject<UInputMappingContext>(nullptr, KeybindContextPath);
    if (!Context)
    {
        UE_LOG(LogEclipseKeybinds, Warning, TEXT("[KEYBIND] IMC_Default missing, cannot apply bindings"));
        return;
    }

    for (const TPair<FName, FKey>& Pair : Bindings)
    {
        const FKey* DefaultKey = Defaults.Find(Pair.Key);
        if (!DefaultKey || *DefaultKey == Pair.Value)
        {
            continue;
        }

        const TCHAR* AssetPath = ActionAssetPath(Pair.Key);
        if (!AssetPath || AssetPath[0] == TEXT('\0'))
        {
            continue;
        }

        UInputAction* Action = LoadObject<UInputAction>(nullptr, AssetPath);
        if (!Action)
        {
            continue;
        }

        Context->UnmapKey(Action, *DefaultKey);
        Context->MapKey(Action, Pair.Value);
        UE_LOG(LogEclipseKeybinds, Log, TEXT("[KEYBIND] applied %s -> %s"), *Pair.Key.ToString(), *Pair.Value.GetDisplayName().ToString());
    }
}

void UEclipseKeybindSubsystem::SaveBindings()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        UEclipseKeybindSaveGame* Save = Cast<UEclipseKeybindSaveGame>(UGameplayStatics::CreateSaveGameObject(UEclipseKeybindSaveGame::StaticClass()));
        if (Save)
        {
            Save->Bindings = Bindings;
            UGameplayStatics::SaveGameToSlot(Save, SaveSlotName(), 0);
            UE_LOG(LogEclipseKeybinds, Log, TEXT("[KEYBIND] saved %d bindings"), Bindings.Num());
        }
    }
}

void UEclipseKeybindSubsystem::LoadBindings()
{
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName(), 0))
    {
        UEclipseKeybindSaveGame* Save = Cast<UEclipseKeybindSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName(), 0));
        if (Save)
        {
            for (const TPair<FName, FKey>& Pair : Save->Bindings)
            {
                if (Bindings.Contains(Pair.Key))
                {
                    Bindings[Pair.Key] = Pair.Value;
                }
            }
            UE_LOG(LogEclipseKeybinds, Log, TEXT("[KEYBIND] loaded %d bindings"), Save->Bindings.Num());
        }
    }
}
