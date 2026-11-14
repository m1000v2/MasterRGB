# MasterRGB 0.2.3 — notas de correção

### O que foi corrigido
1. **Nome da janela:** reaplicado periodicamente durante a inicialização para impedir que a UI original redefina o título para OpenRGB.
2. **Página Sobre:** texto, aba lateral e metadados passam a apresentar MasterRGB 0.2.3. A atribuição ao motor OpenRGB continua preservada por licença.
3. **Aba Informações:** a navegação lateral continua listando páginas técnicas de cada dispositivo e a página Sobre. Isso é intencional no layout base: a aba superior Informações já é a página ativa; a lista lateral é uma subnavegação, não uma repetição da aba Dispositivos.
4. **Tema:** agora existe um seletor real dentro de Configurações > Aparência. A mudança é aplicada imediatamente e gravada localmente.
5. **Codificação:** o C++ gerado é salvo com BOM UTF-8 para que palavras como “doação”, “repositório” e “atualizações” sejam exibidas corretamente.
6. **Apoiar:** a aba segue disponível. Para abrir o PayPal é necessário inserir e salvar o link real do mantenedor.

### O que testar
- Fechar o MasterRGB anterior e compilar este Build Kit.
- Conferir a barra de título e a aba Sobre.
- Abrir Configurações > Aparência, trocar para Claro e depois Escuro sem reiniciar.
- Abrir Apoiar e confirmar que os acentos aparecem corretamente.
- Clicar nos dispositivos em Dispositivos e conferir se o seletor HSV continua funcionando.
