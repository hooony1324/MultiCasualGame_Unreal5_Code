
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Chat.generated.h"

/**
 * 
 */
UCLASS()
class MULTICASUAL_API UChat : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	void AddChatToHistory(const FString& Message);
	TSharedPtr<class SWidget> GetChatInputTextObject();
	void SetKeyboardFocus();


private:
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ChatHistory;
	
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* ChatInputText;

	UFUNCTION()
	void OnChatTextCommited(const FText& Text, ETextCommit::Type CommitMethod);

	UPROPERTY()
	class ALobbyHUD* LobbyHUD;

	UPROPERTY()
	class AMultiCasualPlayerController* PlayerController;
};
