---
hide:
  - toc
---

<div id="scalar-api-reference"></div>

<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/@scalar/api-reference/dist/style.css" />
<script src="https://cdn.jsdelivr.net/npm/@scalar/api-reference"></script>
<script>
  let scalarInstance = null;
  const observer = new MutationObserver(() => {
    // Recreate if theme changes
    renderScalar();
  });
  function renderScalar() {
    if (scalarInstance) {
      observer.disconnect();
      scalarInstance.destroy();
    }
    scalarInstance = Scalar.createApiReference('#scalar-api-reference', {
      url: new URL('../openapi-symbol.yml', window.location.href).toString(),
      slug: 'openapi-symbol',
      title: 'Symbol REST API',
      theme: 'purple',
      showDeveloperTools: 'never',
      hideSearch: true,
      hideClientButton: true,
      agent: { disabled: true },
      mcp: { disabled: true },
      telemetry: false,
      generateTagSlug: (tag) => `${tag.name.replaceAll(' ', '_')}`,
      generateOperationSlug: (operation) => `${operation.operationId.replaceAll('/', '')}`,
      hideDarkModeToggle: true,
      forceDarkModeState: document.body.getAttribute('data-md-color-scheme') === 'slate' ? 'dark' : 'light',
      onLoaded: () => {
        observer.observe(document.body, {
          attributes: true,
          attributeFilter: ['data-md-color-scheme']
        });
      }
    })
  };
  renderScalar();
</script>
<style>
#scalar-api-reference {
  /* Under the header */
  position: relative;
  z-index: 0;
}
#scalar-api-reference [id] {
  /* So it shows below the header */
  scroll-margin-top: 3rem;
}
.md-main__inner {
  /* Use all available width */
  max-width: none;
  margin: 0;
}
</style>
