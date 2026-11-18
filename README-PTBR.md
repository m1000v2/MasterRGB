# MasterRGB 0.3.0 — Build Kit

MasterRGB é uma adaptação do OpenRGB (GPL-2.0) para Windows, com identidade própria,
páginas em português e instalador nativo. Este pacote contém o código de integração e
os scripts de compilação; a base OpenRGB é baixada automaticamente.

## Páginas do aplicativo

- **Início** — painel com os dispositivos detectados e controle de perfis: escolha um perfil salvo e aplique sem sair da tela.
- **Dispositivos** — controles RGB de cada dispositivo (motor original do OpenRGB).
- **Informações** — dados de cada dispositivo na lista lateral; "Sobre o MasterRGB" mostra versão, Qt, Windows e o link do projeto.
- **Configurações** — em Geral > Tema da interface, o tema Claro/Escuro/Automático é aplicado na hora e salvo em `%APPDATA%\MasterRGB\theme.txt`.
- **Apoiar** — doação voluntária por PayPal (com QR Code) e link do projeto. Nenhum campo para preencher e nada é enviado automaticamente.

## Atualizações

O aplicativo consulta as releases públicas do GitHub ao abrir e avisa quando existe
versão nova, com botão para baixar. O download e a instalação só acontecem com a sua
confirmação; nada é atualizado em segundo plano.

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
compila e gera `dist\MasterRGB-0.4.0-Portable.zip`. Para gerar o instalador, compile
`installer\MasterRGB.iss` com o Inno Setup 6.

O caminho de build fica em `%USERPROFILE%\Documents\MasterRGB-Build`. Os perfis são
`.orp` e ficam em `%APPDATA%\OpenRGB`.

## Apoiar

Os links ficam em `config/support.json`. O GitHub e a doação por PayPal já estão
configurados. O QR Code exibido na aba Apoiar é `assets/paypal-qr.png`; se o link mudar,
gere a imagem novamente a partir da nova URL.

## Segurança e licença

A base WinRing0 é legada e possui vulnerabilidade conhecida. Não desative Defender,
Integridade da Memória ou bloqueios de driver para forçar o carregamento. Preserve
GPL-2.0, `LICENSE` e `NOTICE.md` ao redistribuir.
