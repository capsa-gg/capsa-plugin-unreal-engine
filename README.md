# Capsa Unreal Engine plugin

> Open-source, cloud-native logging solution built specifically for Unreal Engine projects. Designed to integrate effortlessly with Unreal Engine to enhance debugging, monitoring, and observability without disrupting your workflow.

This is the Unreal Plugin for capturing and uploading Logs to a Capsa Server. You will need a running Capsa Server (see [capsa-gg/capsa](https://github.com/capsa-gg/capsa)).

![GitHub License](https://img.shields.io/github/license/capsa-gg/capsa-plugin-unreal-engine)

## Key Features
- **Plug & Play**: minimal setup required, works out of the box
- **Cloud-Native**: deploy anywhere — in the cloud or on-premise
- **Optimized for Unreal Engine**: extends Unreal Engine's logging without replacing it
- **Lightweight & Non-Intrusive**: keeps overhead minimal for optimal performance

## Installation and configuration

**Instructions for installing the plugin can be found on _[this](https://capsa.gg/docs/getting-started/unreal-engine-plugin)_ documentation page.**

---

## Getting started with Capsa development

The documentation below contains the Capsa development instructions.

* Clone the repo to your Unreal Project. You can use the [Development Game](https://github.com/capsa-gg/capsa-ue5-dev-game) if you like. 
  If you are in the root of your project, you can run `git clone https://github.com/capsa-gg/capsa-plugin-unreal-engine.git .\Plugins\Capsa`
  Be sure to name the Plugin folder `Capsa` and not `capsa-plugin-unreal-engine`.
* Compile and launch the Editor.
* Modify the `ProjectSettings`->`Engine`->`Capsa`.
* Simply use `UE_LOG` in C++ or `Print String` in Blueprints and these will be uploaded to your Capsa Server.

Documentation about all configuration options can be found [here](https://capsa.gg/docs/configuration/plugin-config).

## Overriding environment variables

In case your setup requires overwrites of certain configuration values for different build types, there are two options.

You can add configuration files that override the base config, as explained [here](https://dev.epicgames.com/documentation/en-us/unreal-engine/configuration-files-in-unreal-engine#configurationfilehierarchy) in the UE docs.

Alternatively, you can use UAT ini-overrides. Overrides take the form of `-ini:Engine:[SettingsKey]:Variable=Value`. So for example to overwrite the `CapsaEnvironmentKey`:

```ps1
.\Path\To\RunUAT.bat BuildCookRun <BuildArgs> -ini:Engine:[/Script/CapsaCore.CapsaSettings]:CapsaEnvironmentKey=<YourEnvironmentKey>
```

## Enabling Verbose and VeryVerbose logging

If, for debugging purposes, you desire to have more verbose logging for certain categories, this can be done in the `DefaultEngine.ini` file, under the `[Core.Log]` section. For example:

```ini
[Core.Log]
; This is used for Capsa plugin development
LogCapsaCore=All
LogCapsaLog=All
; This increases verbosity of UE to generate more log lines for testing
LogNet=Verbose
LogInit=All
LogConfig=Verbose
PIE=Verbose
Cmd=All
```

## Enabling in Shipping

Enabling logging in Shipping comes with risks. It is recommended you research and understand these risks before enabling logging in Shipping builds. There is no guarantee this will work flawlessly or require additional steps.

To enable logging in shipping builds, you need to modify your projects `<ProjectName>.target.cs` file.

In the constructor add:
```csharp
bUseLoggingInShipping = true;
```

Additionally, if building from source, also add:
```csharp
BuildEnvironment = TargetBuildEnvironment.Unique
```

Or if building from precompiled binaries, add:
```csharp
bOverrideBuildEnvironment = true;
```

## Development guidelines

- Primarily use Unreal Code rules, guidelines, formatting etc.
- Headers have tab alignment.
- All properties, functions, classes etc. should have comments, including @param and @return, where appropriate.

### Release steps

Tagged releases should be accompanied by a release of the web stack with the same version number.

- Test with the latest version of the web stack's `main` branch
- Update the versions in the plugin code
- Tag with same version as the new web stack release
- Push to the repository