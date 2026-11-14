# MasterRGB 0.2.3 — Preview Build Kit

Esta revisão corrige os problemas de identidade, codificação e tema relatados no teste da 0.2.2. O pacote é código-fonte e scripts de compilação; não é um executável pré-compilado.

## Mudanças
- Título e página Sobre com identidade MasterRGB.
- A subaba "Sobre o MasterRGB" aparece dentro de Informações. A lista lateral dessa área é subnavegação técnica por dispositivo e permanece disponível por projeto do OpenRGB.
- Seletor de tema funcional em **Configurações > Aparência**, aplica na hora e salva a preferência.
- Texto português da aba Apoiar escrito em UTF-8 com BOM para evitar mojibake no MSVC.
- Botão de doação usa o link PayPal HTTPS configurado pelo usuário; o campo permanece vazio até inserir o endereço real.
- Rebuild limpa apenas os arquivos de UI/branding alterados por versões anteriores, preservando o registro da Red Devil RX 6700 XT.

## Requisitos
- Windows 10/11 x64.
- Git for Windows.
- Visual Studio 2022/2026 com Desktop development with C++, MSVC v143 14.4x e Windows SDK.
- Qt 6.8.3 `msvc2022_64` em `C:\Qt\6.8.3\msvc2022_64`.
- JOM opcional; se não estiver instalado, o script usa NMAKE do MSVC v143.
- Inno Setup 6 opcional para criar instalador.

## Compilar
Extraia o pacote, abra PowerShell na pasta raiz `MasterRGB_v0.2.3` e execute:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\scripts\Build-MasterRGB.ps1
```

O script restaura os arquivos originais de interface/branding do clone e reaplica os patches de identidade, Red Devil e registro do último perfil carregado. O caminho de build continua em `%USERPROFILE%\Documents\MasterRGB-Build`. Os perfis desta branch são `.orp` e ficam em `%APPDATA%\OpenRGB`.

## Tema
Abra **Configurações > Aparência**. A escolha Escuro/Claro aplica imediatamente e fica salva em `%APPDATA%\MasterRGB\theme.txt`.

## Apoiar
Abra a aba **Apoiar**, cole o link PayPal.me ou PayPal.com real e clique em Salvar links. O link antigo do projeto YT Desk Monitor não foi recuperado dos arquivos consultados nesta sessão; por isso ele não foi inventado.

## Segurança e licença
A base WinRing0 é legada e possui vulnerabilidade conhecida. Não desative Defender, Integridade da Memória ou bloqueios de driver para forçar o carregamento. Preservar GPL-2.0, `LICENSE` e `NOTICE.md` ao redistribuir.


## Compilação automática no GitHub

O workflow `.github/workflows/windows-build.yml` prepara Qt 6.8.3, compila no runner Windows 2022, empacota a versão portátil, gera o instalador Inno Setup e calcula SHA-256. Uma execução manual gera artefatos na página Actions. Uma tag `v*` também publica os arquivos no repositório público de Releases quando `MASTER_RGB_RELEASE_TOKEN` estiver configurado no repositório privado.

## Último perfil e inicialização

Esta branch salva perfis no formato `.orp` em `%APPDATA%\OpenRGB`. Depois que um perfil é carregado com sucesso pela interface ou pelo comando de inicialização, o núcleo registra seu nome em `MasterRGB_LastProfile.txt`. A tarefa opcional `MasterRGB Startup` lê esse registro no próximo logon, inicia o programa com o perfil e escreve o resultado em `%APPDATA%\MasterRGB\startup.log`. A restauração precisa ser validada no hardware de teste; a linha `CONFIRMADO` indica que o núcleo atualizou o registro durante a carga.
