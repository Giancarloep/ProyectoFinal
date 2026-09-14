# Sincroniza el frontend editado (web/) a la carpeta que sirve el exe.
# Uso: .\sync-web.ps1   (desde la raiz del proyecto)
# No reinicia el servidor: httplib lee los archivos del disco en cada request.
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$src = Join-Path $root "web"
$dst = Join-Path $root "cmake-build-debug\web"
if (-not (Test-Path $src)) { Write-Host "No existe: $src"; exit 1 }
if (-not (Test-Path $dst)) { New-Item -ItemType Directory -Path $dst | Out-Null }
Copy-Item -Path (Join-Path $src "*") -Destination $dst -Recurse -Force
$files = @("index.html", "css\estilos.css", "js\app.js", "js\auth.js", "js\firebase-config.js", "data\guia-ejercicios.json")
$ok = $true
foreach ($f in $files) {
  $a = Get-FileHash (Join-Path $src $f)
  $b = Get-FileHash (Join-Path $dst $f)
  if ($a.Hash -ne $b.Hash) { Write-Host "DIFERENTE: $f"; $ok = $false }
}
if ($ok) { Write-Host "SYNC OK - recarga el navegador con Ctrl+F5" } else { Write-Host "SYNC FALLO"; exit 1 }
