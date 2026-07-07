# Workspace Rules

- **Terminal Environment Path:** When operating in this workspace, the default terminal environment `Path` variable may be truncated or corrupted, rendering standard utilities like `git` and `npm` unrecognizable. Before running any `git`, `npm`, or similar system commands, you MUST prepend the following to your command to restore the PATH: `$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User");`.
- **Committing Changes:** On the completion of each agent turn or task, you must automatically commit and push all changes to the GitHub repository using the restored PATH above, so that the remote build servers can pick up the deployment.
