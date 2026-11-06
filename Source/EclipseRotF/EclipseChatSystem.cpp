// Eclipse Chat System Implementation

#include "EclipseChatSystem.h"
#include "Engine/Engine.h"

UEclipseChatSystem::UEclipseChatSystem()
{
    // Auto-join World and Area channels
    JoinedChannels.Add(EChatChannel::World, true);
    JoinedChannels.Add(EChatChannel::Area, true);
    JoinedChannels.Add(EChatChannel::System, true);
}

void UEclipseChatSystem::SendMessage(EChatChannel Channel, const FString& Message)
{
    if (!JoinedChannels.Contains(Channel) || !JoinedChannels[Channel])
    {
        UE_LOG(LogTemp, Warning, TEXT("Not joined to channel %d"), (int32)Channel);
        return;
    }

    FChatMessage Msg;
    Msg.Message = Message;
    Msg.Channel = Channel;
    Msg.Timestamp = FDateTime::Now();

    if (!ChannelMessages.Contains(Channel))
    {
        ChannelMessages.Add(Channel, TArray<FChatMessage>());
    }

    ChannelMessages[Channel].Add(Msg);

    // Trim to max
    if (ChannelMessages[Channel].Num() > MaxMessagesPerChannel)
    {
        ChannelMessages[Channel].RemoveAt(0);
    }

    UE_LOG(LogTemp, Log, TEXT("[%s] %s"), *UEnum::GetValueAsString(Channel), *Message);
}

void UEclipseChatSystem::SendWhisper(const FString& TargetPlayer, const FString& Message)
{
    FChatMessage Msg;
    Msg.SenderName = "You";
    Msg.Message = Message;
    Msg.Channel = EChatChannel::Whisper;
    Msg.Timestamp = FDateTime::Now();

    if (!ChannelMessages.Contains(EChatChannel::Whisper))
    {
        ChannelMessages.Add(EChatChannel::Whisper, TArray<FChatMessage>());
    }
    ChannelMessages[EChatChannel::Whisper].Add(Msg);

    UE_LOG(LogTemp, Log, TEXT("[Whisper to %s] %s"), *TargetPlayer, *Message);
}

void UEclipseChatSystem::SendPartyMessage(const FString& Message)
{
    SendMessage(EChatChannel::Party, Message);
}

TArray<FChatMessage> UEclipseChatSystem::GetChannelMessages(EChatChannel Channel, int32 MaxCount)
{
    if (!ChannelMessages.Contains(Channel))
    {
        return TArray<FChatMessage>();
    }

    TArray<FChatMessage> All = ChannelMessages[Channel];
    if (All.Num() <= MaxCount)
    {
        return All;
    }

    // Return last MaxCount messages
    TArray<FChatMessage> Result;
    for (int32 i = All.Num() - MaxCount; i < All.Num(); i++)
    {
        Result.Add(All[i]);
    }
    return Result;
}

void UEclipseChatSystem::JoinChannel(EChatChannel Channel)
{
    JoinedChannels.Add(Channel, true);
    UE_LOG(LogTemp, Log, TEXT("Joined channel: %d"), (int32)Channel);
}

void UEclipseChatSystem::LeaveChannel(EChatChannel Channel)
{
    JoinedChannels.Add(Channel, false);
    UE_LOG(LogTemp, Log, TEXT("Left channel: %d"), (int32)Channel);
}

void UEclipseChatSystem::SetCurrentArea(const FString& AreaName)
{
    CurrentArea = AreaName;
    UE_LOG(LogTemp, Log, TEXT("Area chat switched to: %s"), *AreaName);
}

void UEclipseChatSystem::BindKey(const FString& KeyName, EChatChannel Channel)
{
    Keybinds.Add(KeyName, Channel);
    UE_LOG(LogTemp, Log, TEXT("Bound key %s to channel %d"), *KeyName, (int32)Channel);
}

void UEclipseChatSystem::SetMacro(int32 Slot, const FString& MacroText)
{
    Macros.Add(Slot, MacroText);
    UE_LOG(LogTemp, Log, TEXT("Macro %d set: %s"), Slot, *MacroText);
}

void UEclipseChatSystem::UseMacro(int32 Slot)
{
    if (Macros.Contains(Slot))
    {
        FString Msg = Macros[Slot];
        // Send to area chat by default
        SendMessage(EChatChannel::Area, Msg);
    }
}
