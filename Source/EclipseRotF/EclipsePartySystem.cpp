// Eclipse Party/Raid System Implementation
// 5-player dungeons, 10-player raids, AI + human slots, seamless replacement

#include "EclipsePartySystem.h"
#include "Engine/Engine.h"

UEclipsePartySystem::UEclipsePartySystem()
{
    InitializeGroup(EGroupType::Solo);
}

void UEclipsePartySystem::InitializeGroup(EGroupType Type)
{
    GroupType = Type;
    
    switch (Type)
    {
    case EGroupType::Solo:        MaxSlots = 1;  break;
    case EGroupType::Party:        MaxSlots = 5;  break;
    case EGroupType::Raid:         MaxSlots = 10; break;
    case EGroupType::Arena1v1:     MaxSlots = 1;  break;
    case EGroupType::Arena2v2:     MaxSlots = 2;  break;
    case EGroupType::Arena3v3:     MaxSlots = 3;  break;
    case EGroupType::Arena5v5:     MaxSlots = 5;  break;
    case EGroupType::Battleground: MaxSlots = 10; break;
    }
    
    Members.SetNum(MaxSlots);
    
    // Initialize all slots as empty
    for (int32 i = 0; i < MaxSlots; i++)
    {
        Members[i].SlotIndex = i;
        Members[i].SlotType = EPartySlot::Empty;
        Members[i].bIsAI = true;
        Members[i].bIsAlive = true;
        Members[i].bReady = false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("[Party] Group initialized: %s (%d slots)"), 
        *UEnum::GetValueAsString(Type), MaxSlots);
}

bool UEclipsePartySystem::AddHumanPlayer(int32 Slot, const FString& PlayerName, EClass Class, EClassRole Role)
{
    if (Slot < 0 || Slot >= MaxSlots) return false;
    if (Members[Slot].SlotType != EPartySlot::Empty) return false;
    
    Members[Slot].SlotType = EPartySlot::Player;
    Members[Slot].Name = PlayerName;
    Members[Slot].CharacterClass = Class;
    Members[Slot].Role = Role;
    Members[Slot].bIsAI = false;
    Members[Slot].bIsAlive = true;
    Members[Slot].bReady = false;
    Members[Slot].Health = 100.0f;
    Members[Slot].MaxHealth = 100.0f;
    
    UE_LOG(LogTemp, Log, TEXT("[Party] Human player joined slot %d: %s (%s)"), 
        Slot, *PlayerName, *UEnum::GetValueAsString(Class));
    return true;
}

bool UEclipsePartySystem::AddAICompanion(int32 Slot, const FCompanionProfile& Profile)
{
    if (Slot < 0 || Slot >= MaxSlots) return false;
    if (Members[Slot].SlotType != EPartySlot::Empty) return false;
    
    Members[Slot].SlotType = EPartySlot::AICompanion;
    Members[Slot].Name = Profile.Name;
    Members[Slot].CharacterClass = Profile.CharacterClass;
    Members[Slot].Role = Profile.Role;
    Members[Slot].bIsAI = true;
    Members[Slot].bIsAlive = true;
    Members[Slot].bReady = true;
    Members[Slot].Health = Profile.Health;
    Members[Slot].MaxHealth = Profile.MaxHealth;
    Members[Slot].CompanionProfile = Profile;
    
    UE_LOG(LogTemp, Log, TEXT("[Party] AI companion joined slot %d: %s (%s, %s)"), 
        Slot, *Profile.Name, *UEnum::GetValueAsString(Profile.CharacterClass), *UEnum::GetValueAsString(Profile.Role));
    return true;
}

bool UEclipsePartySystem::RemoveFromSlot(int32 Slot)
{
    if (Slot < 0 || Slot >= MaxSlots) return false;
    
    bool WasAI = Members[Slot].bIsAI;
    FString Name = Members[Slot].Name;
    
    Members[Slot].SlotType = EPartySlot::Empty;
    Members[Slot].Name = "";
    Members[Slot].bIsAI = true;
    Members[Slot].bIsAlive = true;
    Members[Slot].bReady = false;
    
    UE_LOG(LogTemp, Log, TEXT("[Party] Slot %d cleared (was %s: %s)"), 
        Slot, WasAI ? TEXT("AI") : TEXT("Human"), *Name);
    return true;
}

bool UEclipsePartySystem::ReplaceAIWithHuman(int32 Slot, const FString& PlayerName)
{
    if (Slot < 0 || Slot >= MaxSlots) return false;
    if (Members[Slot].SlotType != EPartySlot::AICompanion) return false;
    
    FString OldAIName = Members[Slot].Name;
    EClass Class = Members[Slot].CharacterClass;
    EClassRole Role = Members[Slot].Role;
    
    Members[Slot].SlotType = EPartySlot::Player;
    Members[Slot].Name = PlayerName;
    Members[Slot].bIsAI = false;
    Members[Slot].bReady = false;
    
    UE_LOG(LogTemp, Log, TEXT("[Party] AI %s replaced by human %s in slot %d (same class/role preserved)"), 
        *OldAIName, *PlayerName, Slot);
    return true;
}

bool UEclipsePartySystem::ReplaceHumanWithAI(int32 Slot)
{
    if (Slot < 0 || Slot >= MaxSlots) return false;
    if (Members[Slot].SlotType != EPartySlot::Player) return false;
    
    FString OldPlayerName = Members[Slot].Name;
    
    Members[Slot].SlotType = EPartySlot::AICompanion;
    Members[Slot].bIsAI = true;
    Members[Slot].bReady = true;
    Members[Slot].Name = Members[Slot].CompanionProfile.Name.IsEmpty() ? TEXT("AI Companion") : Members[Slot].CompanionProfile.Name;
    
    UE_LOG(LogTemp, Log, TEXT("[Party] Human %s disconnected, slot %d reverted to AI %s"), 
        *OldPlayerName, Slot, *Members[Slot].Name);
    return true;
}

TArray<FEclipsePartyMember> UEclipsePartySystem::GetPartyMembers() const
{
    return Members;
}

int32 UEclipsePartySystem::GetPartySize() const
{
    int32 Count = 0;
    for (const auto& M : Members)
    {
        if (M.SlotType != EPartySlot::Empty) Count++;
    }
    return Count;
}

int32 UEclipsePartySystem::GetMaxPartySize() const
{
    return MaxSlots;
}

EGroupType UEclipsePartySystem::GetGroupType() const
{
    return GroupType;
}

FString UEclipsePartySystem::GetRoleDistribution() const
{
    int32 Tanks = 0, Healers = 0, DPS = 0, Support = 0;
    for (const auto& M : Members)
    {
        if (M.SlotType == EPartySlot::Empty) continue;
        switch (M.Role)
        {
        case EClassRole::Tank:    Tanks++;    break;
        case EClassRole::Healer:  Healers++;  break;
        case EClassRole::DPS:     DPS++;      break;
        case EClassRole::Support: Support++;  break;
        default: break;
        }
    }
    return FString::Printf(TEXT("Tanks: %d | Healers: %d | DPS: %d | Support: %d"), Tanks, Healers, DPS, Support);
}

void UEclipsePartySystem::FillEmptySlotsWithAI()
{
    for (int32 i = 0; i < MaxSlots; i++)
    {
        if (Members[i].SlotType == EPartySlot::Empty)
        {
            ERaidRole NeededRole = ERaidRole::MeleeDPS;
            
            // Determine what role this slot should be
            switch (GroupType)
            {
            case EGroupType::Party:
                {
                    // 5-player: 1 Tank, 1 Healer, 3 DPS
                    if (i == 0) NeededRole = ERaidRole::MainTank;
                    else if (i == 1) NeededRole = ERaidRole::MainHealer;
                    else NeededRole = ERaidRole::MeleeDPS;
                    break;
                }
            case EGroupType::Raid:
                {
                    // 10-player: 2 Tanks, 2 Healers, 6 DPS
                    if (i < 2) NeededRole = (i == 0) ? ERaidRole::MainTank : ERaidRole::OffTank;
                    else if (i < 4) NeededRole = (i == 2) ? ERaidRole::MainHealer : ERaidRole::SupportHealer;
                    else NeededRole = (i % 2 == 0) ? ERaidRole::MeleeDPS : ERaidRole::RangedDPS;
                    break;
                }
            }
            
            FCompanionProfile AIProfile = GenerateAICompanion(i, NeededRole);
            AddAICompanion(i, AIProfile);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("[Party] All empty slots filled with AI companions"));
    UE_LOG(LogTemp, Log, TEXT("[Party] Role distribution: %s"), *GetRoleDistribution());
}

FCompanionProfile UEclipsePartySystem::GenerateAICompanion(int32 SlotIndex, ERaidRole Role)
{
    FCompanionProfile Profile;
    Profile.SlotIndex = SlotIndex;
    
    // Name pool - real names, not "Bot 1"
    TArray<FString> Names = {
        TEXT("Kael"), TEXT("Zara"), TEXT("Draven"), TEXT("Lyra"), TEXT("Thane"),
        TEXT("Mira"), TEXT("Rhex"), TEXT("Sable"), TEXT("Vorn"), TEXT("Kira"),
        TEXT("Dorn"), TEXT("Ash"), TEXT("Vega"), TEXT("Roan"), TEXT("Nyx")
    };
    Profile.Name = Names[FMath::Rand() % Names.Num()];
    
    // Random personality
    Profile.Personality = (ECompanionPersonality)FMath::RandRange(0, 7);
    
    // Set class and role based on raid role
    switch (Role)
    {
    case ERaidRole::MainTank:
    case ERaidRole::OffTank:
        Profile.Role = EClassRole::Tank;
        Profile.CharacterClass = (Role == ERaidRole::MainTank) ? EClass::Bulwark : EClass::Dreadknight;
        Profile.RaidRole = Role;
        Profile.Health = 200.0f;
        Profile.MaxHealth = 200.0f;
        break;
    case ERaidRole::MainHealer:
    case ERaidRole::SupportHealer:
        Profile.Role = EClassRole::Healer;
        Profile.CharacterClass = (Role == ERaidRole::MainHealer) ? EClass::Templar : EClass::Oracle;
        Profile.RaidRole = Role;
        Profile.Health = 80.0f;
        Profile.MaxHealth = 80.0f;
        break;
    case ERaidRole::MeleeDPS:
        Profile.Role = EClassRole::DPS;
        Profile.CharacterClass = (EClass)FMath::RandRange(0, 16);
        Profile.RaidRole = Role;
        Profile.Health = 100.0f;
        Profile.MaxHealth = 100.0f;
        break;
    case ERaidRole::RangedDPS:
    case ERaidRole::CasterDPS:
        Profile.Role = EClassRole::DPS;
        Profile.CharacterClass = EClass::Arcanist;
        Profile.RaidRole = Role;
        Profile.Health = 70.0f;
        Profile.MaxHealth = 70.0f;
        break;
    case ERaidRole::Utility:
        Profile.Role = EClassRole::Support;
        Profile.CharacterClass = EClass::Alchemist;
        Profile.RaidRole = Role;
        Profile.Health = 90.0f;
        Profile.MaxHealth = 90.0f;
        break;
    }
    
    Profile.Level = 1;
    Profile.WeaponName = "Iron Greatsword";
    Profile.WeaponLevel = 1;
    
    return Profile;
}

void UEclipsePartySystem::BroadcastCommand(ECompanionCommand Command, FVector Location)
{
    UE_LOG(LogTemp, Log, TEXT("[Party] Broadcasting command %d to all AI companions"), (int32)Command);
    for (auto& M : Members)
    {
        if (M.SlotType == EPartySlot::AICompanion)
        {
            UE_LOG(LogTemp, Log, TEXT("[Party] -> %s: %d"), *M.Name, (int32)Command);
        }
    }
}

int32 UEclipsePartySystem::GetLivingMembersCount() const
{
    int32 Count = 0;
    for (const auto& M : Members)
    {
        if (M.SlotType != EPartySlot::Empty && M.bIsAlive) Count++;
    }
    return Count;
}

bool UEclipsePartySystem::IsPartyWiped() const
{
    return GetLivingMembersCount() == 0;
}

int32 UEclipsePartySystem::GetAIcount() const
{
    int32 Count = 0;
    for (const auto& M : Members)
    {
        if (M.SlotType == EPartySlot::AICompanion) Count++;
    }
    return Count;
}

int32 UEclipsePartySystem::GetHumanCount() const
{
    int32 Count = 0;
    for (const auto& M : Members)
    {
        if (M.SlotType == EPartySlot::Player) Count++;
    }
    return Count;
}

bool UEclipsePartySystem::IsEveryoneReady() const
{
    for (const auto& M : Members)
    {
        if (M.SlotType == EPartySlot::Player && !M.bReady) return false;
    }
    return true;
}

void UEclipsePartySystem::SetMemberReady(int32 Slot, bool bReady)
{
    if (Members.IsValidIndex(Slot))
    {
        Members[Slot].bReady = bReady;
        UE_LOG(LogTemp, Log, TEXT("[Party] %s is %s"), *Members[Slot].Name, bReady ? TEXT("READY") : TEXT("NOT READY"));
    }
}
