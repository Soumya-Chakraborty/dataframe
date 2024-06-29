@echo off
REM Change directory to your project folder
cd /d %~dp0

REM Check for existing rebase-merge directory and remove it if found
if exist .git\rebase-merge (
    echo "Cleaning up previous rebase state..."
    rd /s /q .git\rebase-merge
)

REM Initialize the Git repository (if not already initialized)
git init

REM Add all changes to the staging area
git add .

REM Commit the changes
git commit -m "Updated module"

REM Pull the latest changes from the remote repository and rebase your changes
git pull --rebase origin main

IF %ERRORLEVEL% NEQ 0 (
    echo "Rebase failed, attempting to abort rebase."
    git rebase --abort
    echo "Rebase aborted. Please resolve conflicts manually."
    pause
    exit /b %ERRORLEVEL%
)

REM Push the changes to the remote repository
git push -u origin main

IF %ERRORLEVEL% NEQ 0 (
    echo "Push failed. Please resolve issues manually."
    pause
    exit /b %ERRORLEVEL%
)

echo "Changes successfully pushed to GitHub."
pause