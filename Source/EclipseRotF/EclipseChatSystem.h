// Eclipse Chat System — World Chat, Party Chat, Area Channels
// Per V4: social systems, guild chat, party communication

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseChatSystem.generated.h"

UENUM(BlueprintType)
enum class EChatChannel : uint8
{
    World        UMETA(DisplayName = "World — global chat"),
    Area         UMETA(DisplayName = "Area — current zone chat"),
    Party        UMETA(DisplayName = "Party — group chat (max 10)"),
    Guild        UMETA(DisplayName = "Guild — guild chat"),
    Whisper      UMETA(DisplayName = "Whisper — private message"),
    System       UMETA(DisplayName = "System — server announcements"),
    Trade        UMETA(DisplayName = "Trade — marketplace chat"),
    Local        UMETA(DisplayName = "Local — nearby players only"),
};

USTRUCT(BlueprintType)
struct FChatMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString SenderName;
    UPROPERTY(BlueprintReadWrite) FString Message;
    UPROPERTY(BlueprintReadWrite) EChatChannel Channel;
    UPROPERTY(BlueprintReadWrite) FDateTime Timestamp;
    UPROPERTY(BlueprintReadWrite) int32 SenderLevel = 1;
    UPROPERTY(BlueprintReadWrite) FString SenderGuild = "";
    UPROPERTY(BlueprintReadWrite) FLinearColor SenderColor = FLinearColor::White;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseChatSystem : public UObject
{
    GENERATED_BODY()

public:
    UEclipseChatSystem();

    // Send message to a channel
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Chat")
    void SendMessage(EChatChannel Channel, const FString& Message);

    // Send whisper to specific player
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Chat")
    void SendWhisper(const FString& TargetPlayer, const FString& Message);

    // Get messages for a channel
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Chat")
    TArray<FChatMessage> GetChannelMessages(EChatChannel Channel, int32 MaxCount = 50);

    // Channel management
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Chat")
    void JoinChannel(EChatChannel Channel);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Chat")
    void LeaveChannel(EChatChannel Channel);

    // Party chat
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Chat")
    void SendPartyMessage(const FString& Message);

    // Area chat — auto-joined based on current zone
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Chat")
    void SetCurrentArea(const FString& AreaName);

    // Keybinds
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Chat")
    void BindKey(const FString& KeyName, EChatChannel Channel);

    // Macros — preset messages
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Chat")
    void SetMacro(int32 Slot, const FString& MacroText);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Chat")
    void UseMacro(int32 Slot);

private:
    TMap<EChatChannel, TArray<FChatMessage>> ChannelMessages;
    TMap<EChatChannel, bool> JoinedChannels;
    TMap<int32, FString> Macros;
    TMap<FString, EChatChannel> Keybinds;
    FString CurrentArea = "Solara";
    int32 MaxMessagesPerChannel = 100;
};
