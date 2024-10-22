// Copyright Ilgar Lunin. All Rights Reserved.

#include "SpeechRecognizerInitialize.h"
#include "Async/Async.h"


USpeechRecognizerInitialize* USpeechRecognizerInitialize::SpeechRecognizerInitialize(FSpeechRecognizerParameters params)
{
	USpeechRecognizerInitialize* Action = NewObject<USpeechRecognizerInitialize>();
	Action->params = params;
	return Action;
}


void USpeechRecognizerInitialize::Activate() {
	if (!params.Recognizer) {
		Finished.Broadcast(false);
		return;
	}

	if (params.Recognizer->initialization_in_progress) {
		Finished.Broadcast(false);
		return;
	}

	if (!FPaths::DirectoryExists(params.PathToModel)) {
		Finished.Broadcast(false);
		return;
	}

	TWeakObjectPtr<USpeechRecognizerInitialize> Self = this;

	AsyncThread(
		[Self]() {
			if (Self.IsValid()) {
				Self->params.Recognizer->Initialize(Self->params.PathToModel);
			}
		},
		0, TPri_Normal,
		[Self]() {
			if (Self.IsValid()) {
				Self->Finished.Broadcast(true);
				Self->params.Recognizer->initialization_in_progress = false;
			}
		}
    );
}
