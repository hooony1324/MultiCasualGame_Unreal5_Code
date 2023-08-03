

#include "Chat.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "MultiCasualPlayerController.h"
#include "LobbyHUD.h"

void UChat::NativeConstruct()
{
	Super::NativeConstruct();

	ChatInputText->OnTextCommitted.AddDynamic(this, &UChat::OnChatTextCommited);

}

void UChat::AddChatToHistory(const FString& Message)
{
	UTextBlock* NewChatTextBlock = NewObject<UTextBlock>(ChatHistory);
	NewChatTextBlock->SetText(FText::FromString(Message));

	ChatHistory->AddChild(NewChatTextBlock);
	ChatHistory->ScrollToEnd();
}

TSharedPtr<class SWidget> UChat::GetChatInputTextObject()
{
	return ChatInputText->GetCachedWidget();
}

void UChat::SetKeyboardFocus()
{
	ChatInputText->SetKeyboardFocus();
}

void UChat::OnChatTextCommited(const FText& Text, ETextCommit::Type CommitMethod)
{
	PlayerController = PlayerController == nullptr ? 
		Cast<AMultiCasualPlayerController>(GetWorld()->GetFirstPlayerController()) : PlayerController;

	bool bIsLobby = PlayerController->IsLobby();

	switch (CommitMethod)
	{
	case ETextCommit::OnEnter:
		if (Text.IsEmpty() == false)
		{
			PlayerController->SendMessage(Text);
			ChatInputText->SetText(FText::GetEmpty());
		}
		PlayerController->FocusGame();
		break;
	case ETextCommit::OnCleared:
		PlayerController->FocusGame();
		break;
	case ETextCommit::OnUserMovedFocus:
		if (bIsLobby)
		{
			LobbyHUD = LobbyHUD == nullptr ? PlayerController->GetHUD<ALobbyHUD>() : LobbyHUD;
			if (!LobbyHUD->IsStartButtonClicked())
			{
				PlayerController->FocusGame();
			}
		}
		else
		{
			//PlayerController->FocusGame();
		}

		break;
	}
}