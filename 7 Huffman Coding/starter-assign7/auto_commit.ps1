<# -----------------------------------------
  auto_commit.ps1  — Windows twin of auto_commit.sh

  Usage examples:
    -- build hook (no app launch):
    powershell -NoProfile -ExecutionPolicy Bypass -File ".\auto_commit.ps1" --build-hook "REPO_ROOT" "TARGET" "DESTDIR"

    -- run mode (launch + capture):
    powershell -NoProfile -ExecutionPolicy Bypass -File ".\auto_commit.ps1" --run "REPO_ROOT" "TARGET" "DESTDIR"
    powershell -NoProfile -ExecutionPolicy Bypass -File ".\auto_commit.ps1" --run --force-run --exe "C:\path\app.exe" "REPO_ROOT" "TARGET" "DESTDIR"
----------------------------------------- #>

# ================================
# Course settings (customize)
# ================================
$STUDENT="jackylaw"
$ASSIGNMENT = "7"

Set-StrictMode -Version 2
$ErrorActionPreference = "Stop"

# Make Git warnings (stderr) come through stdout so PS5 doesn't treat them as errors
$env:GIT_REDIRECT_STDERR = '2>&1'

# ================================
# Mode/flags + arg parsing
# ================================
$MODE         = $null
$FORCE_RUN    = $false
$EXE_OVERRIDE = ""

$argvQueue  = [System.Collections.Generic.Queue[string]]::new([string[]]$args)
$positional = New-Object System.Collections.Generic.List[string]
while ($argvQueue.Count -gt 0) {
  $tok = $argvQueue.Dequeue()
  switch -Regex ($tok) {
    '^--run$'         { $MODE = '--run' }
    '^--build-hook$'  { $MODE = '--build-hook' }
    '^--force-run$'   { $FORCE_RUN = $true }
    '^--always$'      { $FORCE_RUN = $true }
    '^--no-skip$'     { $FORCE_RUN = $true }
    '^--exe$'         { if ($argvQueue.Count -gt 0) { $EXE_OVERRIDE = $argvQueue.Dequeue() } }
    default           { $positional.Add($tok) }
  }
}
if (-not $MODE) { $MODE = '--run' }

$REPO_ROOT   = if ($positional.Count -ge 1) { $positional[0] } else { (Get-Location).Path }
$TARGET_NAME = if ($positional.Count -ge 2) { $positional[1] } else { "" }
$DESTDIR     = if ($positional.Count -ge 3) { $positional[2] } else { $REPO_ROOT }

Set-Location -LiteralPath $REPO_ROOT

# ================================
# Helpers
# ================================
function Log([string]$msg) { Write-Host "[auto_commit] $msg" }

function Require-Git {
  if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    throw "git not found on PATH."
  }
}

function Head-Exists {
  cmd /c 'git rev-parse --verify --quiet HEAD 1>nul 2>nul' | Out-Null
  return ($LASTEXITCODE -eq 0)
}

function Ensure-Repo {
  if (-not (Test-Path ".git")) {
    Log "No git repo found. Initializing one..."
    git init               2>&1 | Out-Null
    git config user.name  "CS106B Student"          2>&1 | Out-Null
    git config user.email "jackylaw@stanford.edu"   2>&1 | Out-Null
    git config core.autocrlf true                 2>&1 | Out-Null
  }

  # Ensure a .gitignore exists
  if (-not (Test-Path ".gitignore")) {
@'
# keep output logs, ignore other output files
output/*
!output/welcome_output_*.txt
# common artifacts / caches
*.zip
build/
.qtc_clangd/
.vs/
build*/
**/build*/
**/*.app/
'@ | Out-File -FilePath ".gitignore" -Encoding utf8
  }

  # Seed the very first commit — include .gitignore and NameHash.cpp if present
  if (-not (Head-Exists)) {
    $seed = New-Object System.Collections.Generic.List[string]
    if (Test-Path ".gitignore") { $seed.Add(".gitignore") }

    $namehashCandidates = @("NameHash.cpp", "Sources\NameHash.cpp")
    foreach ($p in $namehashCandidates) {
      if (Test-Path -LiteralPath $p) { $seed.Add($p) }
    }

    if (Test-Path "auto_commit.ps1") { $seed.Add("auto_commit.ps1") }

    if ($seed.Count -gt 0) {
      git add -f -- $seed 2>&1 | Out-Null
      git add .
      git commit -m "Initial commit (seed: .gitignore + NameHash.cpp if present)" 2>&1 | Out-Null
      Log "Seeded initial commit with: $($seed -join ', ')"
    } else {
      Log "Nothing to seed in initial commit."
    }
  }
}

function Watched-Files {
  $candidates = @("NameHash.cpp", "Sources\NameHash.cpp")
  foreach ($p in $candidates) { if (Test-Path -LiteralPath $p) { $p } }
}

function Should-Skip-Run([string[]]$watched) {
  if (-not (Head-Exists)) { return $false }
  if (-not $watched -or $watched.Count -eq 0) { return $false }
  $status = git status --porcelain -- $watched 2>$null
  $joined = ($status -join "")
  return [string]::IsNullOrWhiteSpace($joined)
}

function Find-Executable {
  if ($EXE_OVERRIDE -and (Test-Path -LiteralPath $EXE_OVERRIDE)) {
    return (Resolve-Path $EXE_OVERRIDE).Path
  }
  $exe = $null
  if ([string]::IsNullOrWhiteSpace($TARGET_NAME)) {
    $cand = Get-ChildItem -LiteralPath $DESTDIR -File -Filter *.exe -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($cand) { $exe = $cand.FullName }
  } else {
    $candidate = Join-Path $DESTDIR ($TARGET_NAME + ".exe")
    if (Test-Path -LiteralPath $candidate) {
      $exe = (Resolve-Path $candidate).Path
    } else {
      $cand = Get-ChildItem -LiteralPath $DESTDIR -File -Filter *.exe -ErrorAction SilentlyContinue | Select-Object -First 1
      if ($cand) { $exe = $cand.FullName }
    }
  }
  if ($null -eq $exe) { return "" } else { return $exe }
}

function Counter-Bump {
  $f = ".autocommit_counter.txt"
  $n = 0
  if (Test-Path -LiteralPath $f) {
    $first = (Get-Content -LiteralPath $f -TotalCount 1 -ErrorAction SilentlyContinue)
    if ($first -match '^[0-9]+$') { $n = [int]$first }
  }
  $n++
  Set-Content -LiteralPath $f -Value $n
  return $n
}

function Ensure-Git-Identity {
  $name  = (& git config user.name  2>$null)
  $email = (& git config user.email 2>$null)
  if (-not $name)  { git config user.name  "Student Name"        2>&1 | Out-Null }
  if (-not $email) { git config user.email "student@example.com" 2>&1 | Out-Null }
}

function Commit-And-Push([int]$Count, [string]$OutputFile, [string[]]$AlsoAdd) {
  Ensure-Git-Identity

  $ts = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
  $commitSource = Split-Path -Leaf (Get-Location)

  if (Test-Path -LiteralPath $OutputFile) { git add -f -- "$OutputFile" 2>&1 | Out-Null }
  git add -- ".autocommit_counter.txt" "auto_commit.ps1" 2>&1 | Out-Null
  foreach ($f in $AlsoAdd) { if (Test-Path -LiteralPath $f) { git add -f -- "$f" 2>&1 | Out-Null } }

  $staged = (& git diff --name-only --cached)
  if (-not $staged -or $staged.Count -eq 0) {
    Log "Nothing staged after force-add. Check post-build paths/quotes."
    return
  }
  Log ("Staged files : {0}" -f (($staged -join ', ')))

  git commit --no-verify -a -m ("Auto-commit (Windows) #{0} from {1} at {2}" -f $Count, $commitSource, $ts)
  if ($LASTEXITCODE -ne 0) {
    Log "git commit failed (see message above)."
    return
  }

  cmd /c 'git remote get-url origin 1>nul 2>nul' | Out-Null
  $hasOrigin = ($LASTEXITCODE -eq 0)

  if ($hasOrigin) {
    Start-Process git -ArgumentList "push", "-u", "origin", "HEAD" -WindowStyle Hidden
    if ($LASTEXITCODE -ne 0) { Log "Push failed (continuing)." }
  } else {
    Log "No 'origin' remote configured; skipping push."
  }
}

function Package-And-Upload([int]$Count) {
  $tsFile    = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
  $stageRoot = Join-Path ${env:TEMP} ("pensieve_stage_{0}" -f ([guid]::NewGuid()))
  New-Item -ItemType Directory -Force -Path $stageRoot | Out-Null

  $outerName = ("submission_{0}_asg{1}_{2}" -f $STUDENT, $ASSIGNMENT, $tsFile)
  $outerDir  = Join-Path $stageRoot $outerName
  $innerDir  = Join-Path $outerDir ("pensieve_{0}" -f $STUDENT)
  New-Item -ItemType Directory -Force -Path $innerDir | Out-Null

  $rcArgs = @(
    "robocopy",
    "`"$REPO_ROOT`"",
    "`"$innerDir`"",
    "/MIR","/NFL","/NDL","/NJH","/NJS","/NP",
    "/XD","build",".qtc_clangd",".vs",
    "/XF","*.zip","*.obj","*.pdb","*.ilk","*.idb","*.exe","*.dll","*.lib","*.exp","*.idx"
  )
  cmd /c ($rcArgs -join ' ') | Out-Null
  # --- ensure .git makes it into staging and is visible ---
$srcGit   = Join-Path $REPO_ROOT ".git"
$stgGit   = Join-Path $innerDir  ".git"

Log ("Source .git exists: {0}" -f (Test-Path -LiteralPath $srcGit))
Log ("Staged .git exists: {0}" -f (Test-Path -LiteralPath $stgGit))

if (Test-Path -LiteralPath $srcGit) {
  if (-not (Test-Path -LiteralPath $stgGit)) {
    try {
      Copy-Item -Recurse -Force -LiteralPath $srcGit -Destination $stgGit
      Log "Manually copied .git into staging."
    } catch {
      Log "ERROR copying .git: $($_.Exception.Message)"
    }
  }

  # Clear Hidden/System so Explorer shows it inside the zip
  try {
    attrib -H -S $stgGit /S /D 2>$null
    Log "Cleared Hidden/System attributes on staged .git."
  } catch {
    Log "WARN: couldn't clear .git attributes: $($_.Exception.Message)"
  }
} else {
  Log "WARN: .git missing at source; nothing to stage."
}

  
  $zipName = "$outerName.zip"
  $zipPath = Join-Path $stageRoot $zipName
  if (Test-Path -LiteralPath $zipPath) { Remove-Item -LiteralPath $zipPath -Force }

  $zipOk = $false
  Push-Location $stageRoot
  try {
    Compress-Archive -Path $outerName -DestinationPath $zipPath -Force -ErrorAction Stop
    $zipOk = $true
  } catch {
    $tar = (Get-Command tar.exe -ErrorAction SilentlyContinue)
    if ($tar) {
      & $tar.Source -a -c -f "$zipPath" "$outerName"
      if ($LASTEXITCODE -eq 0) { $zipOk = $true }
    }
  } finally {
    Pop-Location
  }

try {
  Add-Type -AssemblyName System.IO.Compression.FileSystem
  $zip = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
  $hasGit = $zip.Entries.FullName | Where-Object { $_ -match '/\.git(/|$)' } | Select-Object -First 1
  $zip.Dispose()
  if ($hasGit) { Log ".git confirmed inside zip." } else { Log "WARNING: .git not found inside zip." }
} catch { Log "Zip introspection failed: $($_.Exception.Message)" }


  if (-not $zipOk -or -not (Test-Path -LiteralPath $zipPath)) {
    Log "Zip failed; skipping upload."
    Remove-Item -Recurse -Force $stageRoot -ErrorAction SilentlyContinue
    return
  }

  $curl = (Get-Command curl.exe -ErrorAction SilentlyContinue | Select-Object -First 1).Source
  if (-not $curl) { $curl = Join-Path $env:SystemRoot "System32\curl.exe" }

  if (-not (Test-Path -LiteralPath $curl)) {
    Log "curl.exe not found; skipping upload."
  } else {
    Log ("Uploading {0} to Pensieve..." -f (Split-Path -Leaf $zipPath))
    & $curl --progress-bar `
      -F ("file=@{0}" -f $zipPath) `
      -F ("sunet={0}" -f $STUDENT) `
      -F ("assignment={0}" -f $ASSIGNMENT) `
      "https://pincs.stanford.edu/cgi-bin/pensieve/upload-zip.cgi" |
      Write-Host
    if ($LASTEXITCODE -ne 0) { Log "Upload failed." } else { Log "Upload succeeded." }
  }

  Remove-Item -Recurse -Force $stageRoot -ErrorAction SilentlyContinue
}

# ================================
# Main
# ================================
Require-Git
Ensure-Repo

$WATCH = @()
try { $WATCH = @(Watched-Files) } catch { $WATCH = @() }

if ($MODE -eq '--build-hook') {
  $anyChanges = git status --porcelain 2>$null
  $joined = ($anyChanges -join "")
  if ([string]::IsNullOrWhiteSpace($joined)) {
    Log "no changes after build; skipping commit."
    exit 0
  }

  $COUNT = Counter-Bump
  $OUTPUT_DIR  = Join-Path $REPO_ROOT "output"
  New-Item -ItemType Directory -Force -Path $OUTPUT_DIR | Out-Null
  $OUTPUT_FILE = Join-Path $OUTPUT_DIR ("build_output_{0}.txt" -f $COUNT)

  $targetDisplay = if ([string]::IsNullOrWhiteSpace($TARGET_NAME)) { "<unknown>" } else { $TARGET_NAME }

@"
Build hook auto-commit #$COUNT
$(Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz")
Target : $targetDisplay
Destdir: $DESTDIR

git status (pre-commit):
"@ | Out-File -FilePath $OUTPUT_FILE -Encoding utf8

  (git status --porcelain=v1) | Out-File -FilePath $OUTPUT_FILE -Append -Encoding utf8

  Commit-And-Push -Count $COUNT -OutputFile $OUTPUT_FILE -AlsoAdd $WATCH
  Package-And-Upload -Count $COUNT
  exit 0
}

if (-not $FORCE_RUN -and (Should-Skip-Run $WATCH)) {
  Log "watched sources unchanged; skipping app launch + commit."
  exit 0
}

$COUNT = Counter-Bump
$OUTPUT_DIR  = Join-Path $REPO_ROOT "output"
New-Item -ItemType Directory -Force -Path $OUTPUT_DIR | Out-Null
$OUTPUT_FILE = Join-Path $OUTPUT_DIR ("welcome_output_{0}.txt" -f $COUNT)

$EXE = Find-Executable
if ([string]::IsNullOrWhiteSpace($EXE) -or -not (Test-Path -LiteralPath $EXE)) {
  $shown = if ([string]::IsNullOrWhiteSpace($EXE)) { "<empty>" } else { $EXE }
  throw ("Executable not found or not executable: {0}" -f $shown)
}

$env:SCREENSHOT_MODE = "1"
try {
  New-Item -ItemType File -Force -Path $OUTPUT_FILE | Out-Null
  & "$EXE" 2>&1 | Tee-Object -FilePath $OUTPUT_FILE | Out-Null
} catch {
  "[run error] $($_.Exception.Message)" | Out-File -FilePath $OUTPUT_FILE -Append -Encoding utf8
}

if (-not (Test-Path -LiteralPath $OUTPUT_FILE) -or ((Get-Item $OUTPUT_FILE).Length -eq 0)) {
  "[no output captured]" | Out-File -FilePath $OUTPUT_FILE -Encoding utf8
}

Commit-And-Push -Count $COUNT -OutputFile $OUTPUT_FILE -AlsoAdd $WATCH
Package-And-Upload -Count $COUNT
exit 0

