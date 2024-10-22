// Copyright Ilgar Lunin. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;


public class VoskPlugin : ModuleRules
{
	public VoskPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        string VoskPath = Path.Combine(Path.Combine(PluginDirectory, "ThirdParty", "vosk"));

        PublicIncludePaths.AddRange(
			new string[] {
				  Path.Combine(ModuleDirectory, "Public"),
                  Path.Combine(VoskPath)
            }
		);

        string PlatformBinariesPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "Binaries", Target.Platform.ToString()));

        if (Target.Platform == UnrealTargetPlatform.Win64)
		{

			// let unreal know where to look for dlls
			PublicRuntimeLibraryPaths.Add(VoskPath);
            PrivateRuntimeLibraryPaths.Add(VoskPath);

			try
			{
				File.Copy(Path.Combine(VoskPath, "libvosk.dll"), Path.Combine(PlatformBinariesPath, "libvosk.dll"));
				File.Copy(Path.Combine(VoskPath, "libwinpthread-1.dll"), Path.Combine(PlatformBinariesPath, "libwinpthread-1.dll"));
				File.Copy(Path.Combine(VoskPath, "libstdc++-6.dll"), Path.Combine(PlatformBinariesPath, "libstdc++-6.dll"));
				File.Copy(Path.Combine(VoskPath, "libgcc_s_seh-1.dll"), Path.Combine(PlatformBinariesPath, "libgcc_s_seh-1.dll"));
			} catch (Exception ex)
			{

			}

			// dlls will be copied to packaged build binaries folder next to main executable
            RuntimeDependencies.Add(Path.Combine("$(BinaryOutputDir)", "libvosk.dll"), Path.Combine(VoskPath, "libvosk.dll"));
            RuntimeDependencies.Add(Path.Combine("$(BinaryOutputDir)", "libwinpthread-1.dll"), Path.Combine(VoskPath, "libwinpthread-1.dll"));
            RuntimeDependencies.Add(Path.Combine("$(BinaryOutputDir)", "libstdc++-6.dll"), Path.Combine(VoskPath, "libstdc++-6.dll"));
            RuntimeDependencies.Add(Path.Combine("$(BinaryOutputDir)", "libgcc_s_seh-1.dll"), Path.Combine(VoskPath, "libgcc_s_seh-1.dll"));
            
            // dynamic
            PublicDelayLoadDLLs.Add("libvosk.dll");
            PublicDelayLoadDLLs.Add("libwinpthread-1.dll");
            PublicDelayLoadDLLs.Add("libstdc++-6.dll");
            PublicDelayLoadDLLs.Add("libgcc_s_seh-1.dll");

            // static
            PublicAdditionalLibraries.Add(Path.Combine(VoskPath, "libvosk.lib"));

			
		}

        PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Voice",
				"Json",
				"Projects",
				"WebSockets"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Voice",
				"CoreUObject",
				"Engine"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
