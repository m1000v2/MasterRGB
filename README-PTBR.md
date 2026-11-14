# MasterRGB 0.3.0 — Build Kit

MasterRGB é uma adaptação do OpenRGB (GPL-2.0) para Windows, com identidade própria,
páginas em português e instalador nativo. Este pacote contém o código de integração e
os scripts de compilação; a base OpenRGB é baixada automaticamente.

## Páginas do aplicativo

- **Início** — painel com os dispositivos detectados e atalhos para controles, configurações e atualizações.
- **Dispositivos** — controles RGB de cada dispositivo (motor original do OpenRGB).
- **Informações** — abre em "Sobre o MasterRGB"; os dados de cada dispositivo continuam na lista lateral.
- **Configurações** — em Geral > Tema da interface, o tema Claro/Escuro/Automático é aplicado na hora e salvo em `%APPDATA%\MasterRGB\theme.txt`.
- **Apoiar** — botões prontos para o GitHub e para o PayPal, quando configurado. Não há campos para o usuário preencher.

## Iniciar com o Windows

O instalador oferece a opção **Iniciar o MasterRGB junto com o Windows**. Ela cria
`MasterRGB.lnk` na pasta Inicializar do usuário. O Windows mostra esse item em
**Gerenciador de Tarefas > Aplicativos de inicialização**, onde pode ser desativado.
A mesma opção existe dentro do aplicativo em **Configurações > Geral**.
Nenhuma tarefa agendada e nenhuma janela de PowerShell são usadas.

Quando iniciado pelo Windows, o aplicativo abre minimizado e carrega o último perfil
aplicado no motor RGB (`%APPDATA%\OpenRGB\MasterRGB_LastProfile.txt`).

## Requisitos

- Windows 10/11 x64.
- Git for Windows.
- Visual Studio 2022/2026 com Desktop development with C++, MSVC v143 14.4x e Windows SDK.
- Qt 6.8.3 `msvc2022_64` em `C:\Qt\6.8.3\msvc2022_64`.
- JOM opcional; sem ele o script usa NMAKE do MSVC v143.
- Inno Setup 6 para gerar o instalador.

## Compilar

Extraia o pacote, abra PowerShell na pasta raiz e execute:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\scripts\Build-MasterRGB.ps1
```

O script restaura os arquivos originais de interface, aplica a integração MasterRGB,
compila e gera `dist\MasterRGB-0.3.0-Portable.zip`. Para gerar o instalador, compile
`installer\MasterRGB.iss` com o Inno Setup 6.

O caminho de build fica em `%USERPROFILE%\Documents\MasterRGB-Build`. Os perfis são
`.orp` e ficam em `%APPDATA%\OpenRGB`.

## Apoiar

Os links ficam em `config/support.json`. O GitHub já está configurado. Para doações,
preencha `paypalUrl` com um endereço HTTPS oficial do PayPal; enquanto estiver vazio,
o botão de doação simplesmente não aparece.

## Segurança e licença

A base WinRing0 é legada e possui vulnerabilidade conhecida. Não desative Defender,
Integridade da Memória ou bloqueios de driver para forçar o carregamento. Preserve
GPL-2.0, `LICENSE` e `NOTICE.md` ao redistribuir.
