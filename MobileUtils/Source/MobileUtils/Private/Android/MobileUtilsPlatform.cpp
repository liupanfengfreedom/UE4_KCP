// Mobile Utils Plugin
// Created by Patryk Stepniewski
// Copyright (c) 2014-2019 gameDNA Ltd. All Rights Reserved.

#include "MobileUtilsPlatform.h"
#include "IMobileUtils.h"

jmethodID FMobileUtilsPlatform::SetOrientationMethod;
jmethodID FMobileUtilsPlatform::StartActivityMethod;
jmethodID FMobileUtilsPlatform::DispatchMessageMethod;
jmethodID FMobileUtilsPlatform::GetlocalipaddressMethod;

FMobileUtilsPlatform::FMobileUtilsPlatform()
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		SetOrientationMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_SetOrientation", "(I)V", false);
		StartActivityMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_StartActivity", "(Ljava/lang/String;)V", false);
		DispatchMessageMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_DispatchMessage", "(ILjava/lang/String;)V", false);
		GetlocalipaddressMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_Getlocalipaddress", "()Ljava/lang/String;", false);
	}
}

FMobileUtilsPlatform::~FMobileUtilsPlatform()
{
}

void FMobileUtilsPlatform::SetOrientation(int orientation)
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, FMobileUtilsPlatform::SetOrientationMethod, orientation);
	}
}

void FMobileUtilsPlatform::StartActivity(const FString& activity)
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jstring JStr = Env->NewStringUTF(TCHAR_TO_UTF8(*activity));
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, FMobileUtilsPlatform::StartActivityMethod, JStr);
		Env->DeleteLocalRef(JStr);
	}
}
FString FMobileUtilsPlatform::getlocalipaddress()
{
	FString ResultDeviceId = FString("");
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jstring ResultDeviceIdString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, FMobileUtilsPlatform::GetlocalipaddressMethod);
		const char* nativeDeviceIdString = Env->GetStringUTFChars(ResultDeviceIdString, 0);
		ResultDeviceId = FString(nativeDeviceIdString);
		Env->ReleaseStringUTFChars(ResultDeviceIdString, nativeDeviceIdString);
		Env->DeleteLocalRef(ResultDeviceIdString);
	}
	return ResultDeviceId;
}
void FMobileUtilsPlatform::DispatchMessage(int message, const JsonSharedPtr jsonObject)
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		if (jsonObject.IsValid())
		{
			FString ostr;
			TSharedRef<TJsonWriter<>> write = TJsonWriterFactory<>::Create(&ostr);
			if (FJsonSerializer::Serialize(jsonObject.ToSharedRef(), write))
			{
				jstring JStr = Env->NewStringUTF(TCHAR_TO_UTF8(*ostr));
				FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, FMobileUtilsPlatform::DispatchMessageMethod, message, JStr);
				Env->DeleteLocalRef(JStr);
			}
			else
			{
				UE_LOG(LogEngine, Error, TEXT("\nDispatchMessage json serializer fail.\n"));
			}
		}
		else
		{
			FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, FMobileUtilsPlatform::DispatchMessageMethod, message, NULL);
		}
	}
}

JNI_METHOD void Java_com_epicgames_ue4_GameActivity_nativeOnMessageCallback(JNIEnv* jenv, jclass jc, jint message, jstring json)
{
	if (FTaskGraphInterface::IsRunning())
	{
		if (json != NULL)
		{
			const char *json_uc = jenv->GetStringUTFChars(json, 0);
			FString json_fstr = FString(json_uc);
			jenv->ReleaseStringUTFChars(json, json_uc);

			JsonSharedPtr json_obj = MakeShareable(new FJsonObject);
			TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(json_fstr);
			if (FJsonSerializer::Deserialize(reader, json_obj))
			{
				FFunctionGraphTask::CreateAndDispatchWhenReady([json_obj, message]()
				{
					MessageCallback callback = IMobileUtils::Get().GetPlatformInterface()->GetMessageCallback();
					if (callback != NULL)
						callback(message, json_obj);
				}, TStatId(), NULL, ENamedThreads::GameThread);
			}
			else
			{
				UE_LOG(LogEngine, Error, TEXT("\nNativeOnMessageCallback json deserialize fail.\n"));
			}
		}
		else
		{
			FFunctionGraphTask::CreateAndDispatchWhenReady([message]()
			{
				MessageCallback callback = IMobileUtils::Get().GetPlatformInterface()->GetMessageCallback();
				if (callback != NULL)
					callback(message, NULL);
			}, TStatId(), NULL, ENamedThreads::GameThread);
		}
	}
}

const FString &GetBasePath()
{
	extern FString GFilePathBase;
	static FString BasePath = GFilePathBase + FString("/UE4Game/") + FApp::GetProjectName() + FString("/");
	return BasePath;
}

FString FMobileUtilsPlatform::ConvertToAbsolutePath(const FString& path)
{
	if (FPaths::IsRelative(path))
	{
		FString file = path.Replace(TEXT("../"), TEXT(""), ESearchCase::CaseSensitive);
		return GetBasePath() + file;
	}
	return path;
}