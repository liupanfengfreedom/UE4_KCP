// Fill out your copyright notice in the Description page of Project Settings.

#include "MyBlueprintFunctionLibrary.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Runtime/ImageWrapper/Public/IImageWrapper.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#include "Engine.h"
#include "Runtime/Core/Public/Misc/DateTime.h"
#include "Runtime/Core/Public/Misc/CoreDelegates.h"
#include "Runtime/Core/Public/Misc/SecureHash.h"
#include "Runtime/Core/Public/HAL/UnrealMemory.h"
#include "MobileUtilsBlueprintLibrary.h"
#include "Media/Public/IMediaCaptureSupport.h"
#include "MediaUtils/Public/MediaCaptureSupport.h"
#include "Slate/Public/Framework/Application/SlateApplication.h"
#if PLATFORM_WINDOWS
#include "ApplicationCore/Public/Windows/WindowsPlatformApplicationMisc.h"
#endif
TArray<FString> UMyBlueprintFunctionLibrary::MountedPakList;
TArray<Fonge> UMyBlueprintFunctionLibrary::OnGameInitevent;
TArray<Fonge> UMyBlueprintFunctionLibrary::OnGameexit;
TArray<Fang> UMyBlueprintFunctionLibrary::OnGameexiteventwithparameter;
void UMyBlueprintFunctionLibrary::FStringtoUTF8(FString & in, uint8 *& out, int32& outsize)
{
	TCHAR *arr = in.GetCharArray().GetData();
	out = (uint8*)TCHAR_TO_UTF8(arr);///this macro ofen do not work , "you'd better use this macro directly"
	outsize = FCString::Strlen(arr);
}
FString UMyBlueprintFunctionLibrary::UTF8toFString(const TArray<uint8>& in)
{
	return FString(UTF8_TO_TCHAR(&in[0])).Left(in.Num());
}
void UMyBlueprintFunctionLibrary::FStringtoUTF16(FString & in, uint8 *& out, int64 & outsize)
{
	TCHAR *arr = in.GetCharArray().GetData();
	outsize = in.Len()<<1;
	out = (uint8 *)arr;
}
FString UMyBlueprintFunctionLibrary::UTF16toFString(const TArray<uint8>& in, int64 size)
{
	return FString(size>>1, (TCHAR*)&in[0]);
	//return FString((wchar_t*)&in[0]);
}
void UMyBlueprintFunctionLibrary::readstringfromfile(FString filepath, FString & content)
{
	FFileHelper::LoadFileToString(content,*filepath);
}
void UMyBlueprintFunctionLibrary::readdatafromfile(FString filepath, TArray<uint8> & content)
{
		FFileHelper::LoadFileToArray(content, *filepath);
}
bool UMyBlueprintFunctionLibrary::FileMd5isequalSpecificMd5(FString filepath, FString SpecificMd5)
{
	TArray<uint8> content;
	readdatafromfile(filepath, content);
	FString md51 = FMD5::HashBytes(&content[0], content.Num());
	if (md51.Equals(SpecificMd5,ESearchCase::IgnoreCase))
	{
		return true;
	}
	return false;
}
void UMyBlueprintFunctionLibrary::writedatatofile(FString filepath, const TArray<uint8> & content)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	//PlatformFile.CreateDirectory(*filepath);
	FString FileContent = TEXT("This is a line of text to put in the file.\n");
	FFileHelper::SaveStringToFile(FileContent, *filepath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);

	IFileHandle* FileHandle = PlatformFile.OpenWrite(*filepath);
	if (FileHandle)
	{
		uint8* ByteBuffer = (uint8*)&content[0];
		//int32* IntPointer = &MyInteger;
		//uint8* ByteBuffer = reinterpret_cast<uint8*>(IntPointer);

		// Write the bytes to the file
		FileHandle->Write(ByteBuffer, content.Num());

		// Close the file again
		delete FileHandle;
	}
}
void UMyBlueprintFunctionLibrary::writedatatofile(FString &filepath,  uint8*& ByteBuffer,  int64& size)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	//PlatformFile.CreateDirectory(*filepath);
	FString FileContent = TEXT("This is a line of text to put in the file.\n");
	FFileHelper::SaveStringToFile(FileContent, *filepath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);

	IFileHandle* FileHandle = PlatformFile.OpenWrite(*filepath);
	if (FileHandle)
	{
		// Write the bytes to the file
		FileHandle->Write(ByteBuffer, size);
		// Close the file again
		delete FileHandle;
	}
}
bool UMyBlueprintFunctionLibrary::Mount(FString PakFileName)
{
	if (MountedPakList.Contains(PakFileName))
	{
		return false;
	}
	int PakIndex = 0;
	int PakReadOrder = 0;
	FString Contentdir = FPaths::ProjectContentDir();
	Contentdir.Append(PakFileName);
	bool fb = FPaths::FileExists(Contentdir);
	if (fb)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("PakFileName exist"));

	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("PakFileName not exist"));

		return false;
	}
//	FString AbsPakPath = FPaths::ConvertRelativePathToFull(Contentdir);
    FString AbsPakPath = UMobileUtilsBlueprintLibrary::ConvertToAbsolutePath(Contentdir);
	if (FCoreDelegates::OnMountPak.IsBound())
	{
		auto bSuccess = FCoreDelegates::OnMountPak.Execute(AbsPakPath, PakReadOrder, nullptr);
		if (!bSuccess)
		{
			////GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("sandbox"));

			// This can fail because of the sandbox system - which the pak system doesn't understand.
			auto SandboxedPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*AbsPakPath);
			bSuccess = FCoreDelegates::OnMountPak.Execute(SandboxedPath, PakReadOrder, nullptr);
		}
		if (bSuccess)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("mount ok"));

		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("mount fail"));
			return false;

		}
	}
	MountedPakList.Add(PakFileName);
	return true;

}


bool UMyBlueprintFunctionLibrary::UnMount(FString PakFileName)
{
	if (!MountedPakList.Contains(PakFileName))
	{
		return false;
	}
	int PakIndex = 0;
	int PakReadOrder = 0;
	FString Contentdir = FPaths::ProjectContentDir();
	Contentdir.Append(PakFileName);
	bool fb = FPaths::FileExists(Contentdir);
	if (fb)
	{
		////GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("PakFileName exist"));

	}
	else
	{
		////GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("PakFileName not exist"));

		return false;
	}
//	FString AbsPakPath = FPaths::ConvertRelativePathToFull(Contentdir);
    FString AbsPakPath = UMobileUtilsBlueprintLibrary::ConvertToAbsolutePath(Contentdir);
	if (FCoreDelegates::OnUnmountPak.IsBound())
	{
		auto bSuccess = FCoreDelegates::OnUnmountPak.Execute(AbsPakPath);
		if (!bSuccess)
		{
			////GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("unmount sandbox"));

			// This can fail because of the sandbox system - which the pak system doesn't understand.
			auto SandboxedPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*AbsPakPath);
			bSuccess = FCoreDelegates::OnUnmountPak.Execute(SandboxedPath);
		}
		if (bSuccess)
		{
			////GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("unmount ok"));

		}
		else
		{
			////GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("unmount fail"));
			return false;

		}
	}
	MountedPakList.Remove(PakFileName);
	return true;
}

void UMyBlueprintFunctionLibrary::RawImageToTexture2D(const TArray<uint8> &RawFileData, class UTexture2D *& out_texture)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrappers[3] =
	{
		ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG),
		ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG),
		ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP),
	};
	for (auto ImageWrapper : ImageWrappers)
	{
		if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
		{
			TArray<uint8> UncompressedBGRA;
			if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
			{
				out_texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
				void* TextureData = (uint8*)out_texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
				out_texture->PlatformData->Mips[0].BulkData.Unlock();
				out_texture->UpdateResource();
			}
		}
	}
}
void UMyBlueprintFunctionLibrary::CLogtofile(FString msg)
{
	FFileHelper::SaveStringToFile(msg+"  :"+FDateTime::UtcNow().ToString()+"\n", *(FPaths::ProjectSavedDir()+"Log.log"), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
}
UObject* UMyBlueprintFunctionLibrary::Loadobject(FString path)
{
	UObject * obj = LoadObject<UObject>(nullptr, *path);
	return obj;
}
const FString UMyBlueprintFunctionLibrary::Screenshoot(FString  infilename, bool bInShowUI, bool bAddFilenameSuffix, bool brelativepath)
{
	FScreenshotRequest::RequestScreenshot(infilename, bInShowUI, bAddFilenameSuffix);
	FString path = FScreenshotRequest::GetFilename(); 
	if (brelativepath)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, *path);
		return path;
	}
	else
	{
//		FString fullpath = FPaths::ConvertRelativePathToFull(path);
        FString fullpath = UMobileUtilsBlueprintLibrary::ConvertToAbsolutePath(path);
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, *fullpath);
		return fullpath;
	}
}
TArray<FString> UMyBlueprintFunctionLibrary::getcamerainfor()
{
	TArray<FString> urls;
	TArray<FMediaCaptureDeviceInfo> DeviceInfos;
	MediaCaptureSupport::EnumerateVideoCaptureDevices(DeviceInfos);
	for (auto var : DeviceInfos)
	{
		urls.Add(var.DisplayName.ToString() + "???" + var.Url);
	}
	return urls;
}
FLinearColor UMyBlueprintFunctionLibrary::getcolorinforundercursor()
{
#if PLATFORM_WINDOWS
	FVector2D CurrentCursorPosition = FSlateApplication::Get().GetCursorPos();
	return FPlatformApplicationMisc::GetScreenPixelColor(CurrentCursorPosition, 1.0f/*Gamma*/);
#endif // PLATFORM_WINDOWS
	return FLinearColor();
}
TArray<FString> UMyBlueprintFunctionLibrary::findallfileunderpath(FString path, FString FileExtension)
{
	TArray<FString> files;
	//FPlatformFileManager::Get().GetPlatformFile().FindFilesRecursively(files, L"D:\\ueprojects\\MyProject", L".h");
	FPlatformFileManager::Get().GetPlatformFile().FindFilesRecursively(files, *path, *FileExtension);
	return files;
}
void UMyBlueprintFunctionLibrary::Ongameinitfunc()
{
	//clear log file content
	FFileHelper::SaveStringToFile(FString("log file") + "  :" + FDateTime::UtcNow().ToString() + "\n", *(FPaths::ProjectSavedDir()+"Log.log"), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_None);
	for (auto var : OnGameInitevent)//
	{
		var.ExecuteIfBound();
	}

}

void UMyBlueprintFunctionLibrary::OnGameEndefunc()
{
	for (auto var : OnGameexit)
	{
		var.ExecuteIfBound();
	}
	for (auto var : OnGameexiteventwithparameter)
	{
		var.ExecuteIfBound(TArray<uint8>(), FString("hi param"));
	}
}



