# Capsa

This is the Unreal Plugin for capturing and uploading Logs to a Capsa Server. You will need a running Capsa Server (see https://github.com/capsa-gg/capsa).

## Getting started

Clone the repo to your Unreal Project.
Be sure to name the Plugin folder "Capsa" (not capsa-plugin-unreal-engine).
Compile and launch the Editor.
Modify the ProjectSettings->Engine->Capsa. Minimum requirement is to set the CapsaBaseURL to the accessible address of your Capsa server.
Simply use UE_LOG or PrintString (Blueprint Node) and these will be uploaded to your Capsa Server.

## Development guidelines

Primarily use Unreal Code rules, guidelines, formatting etc.
Headers have tab alignment.
All properties, functions, classes etc should have comments, including @param and @return, where appropriate.
