# Capsa Unreal Engine plugin

> Open-source, cloud-native logging solution built specifically for Unreal Engine projects. Designed to integrate effortlessly with Unreal Engine to enhance debugging, monitoring, and observability without disrupting your workflow.

This is the Unreal Plugin for capturing and uploading Logs to a Capsa Server. You will need a running Capsa Server (see https://github.com/capsa-gg/capsa).

![GitHub License](https://img.shields.io/github/license/capsa-gg/capsa-plugin-unreal-engine)

## Key Features
- **Plug & Play**: minimal setup required, works out of the box
- **Cloud-Native**: deploy anywhere — in the cloud or on-premise
- **Optimized for Unreal Engine**: extends Unreal Engine's logging without replacing it
- **Lightweight & Non-Intrusive**: keeps overhead minimal for optimal performance

---

## Getting started

Instructions for getting started can be found on [capsa.gg](https://capsa.gg/docs/getting-started/).

* Clone the repo to your Unreal Project. 
  If you are in the root of your project, you can run `git clone https://github.com/capsa-gg/capsa-plugin-unreal-engine.git .\Plugins\Capsa`
  Be sure to name the Plugin folder "Capsa" (not capsa-plugin-unreal-engine).
* Compile and launch the Editor.
* Modify the ProjectSettings->Engine->Capsa. Minimum requirement is to set the CapsaBaseURL to the accessible address of your Capsa server.
* Simply use UE_LOG or PrintString (Blueprint Node) and these will be uploaded to your Capsa Server.

## Development guidelines

Primarily use Unreal Code rules, guidelines, formatting etc.
Headers have tab alignment.
All properties, functions, classes etc should have comments, including @param and @return, where appropriate.

## Overriding environment variables

## Enabling Verbose and VeryVerbose logging

TODO `[Core.Log]` in DefaultEngine.ini

## Enabling in Shipping

Enabling logging in Shipping comes with risks. It is recommended you research and understand these risks before enabling logging in Shipping builds. There is no guarantee this will work flawlessly or require additional steps.

To enable logging in shipping builds, you need to modify your projects <ProjectName>.target.cs file.

In the constructor add:
bUseLoggingInShipping = true;

Additionally, if building from source, also add:
BuildEnvironment = TargetBuildEnvironment.Unique

Or if building from precompiled binaries, add:
bOverrideBuildEnvironment = true;

## Release guide

- Testing
- Set version
- Push to public repo