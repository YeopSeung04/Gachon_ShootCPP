using UnrealBuildTool;

public class ShootCPPEditor : ModuleRules
{
	public ShootCPPEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"Kismet",
			"Slate",
			"SlateCore",
			"UMG",
			"UMGEditor",
			"AssetRegistry",
			"ShootCPP"
		});
	}
}
