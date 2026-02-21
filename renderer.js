// Renderer-side logic for Electron integration
// The API base URL defaults to localhost:8080 where the C++ backend runs
(function () {
  const API_BASE = window.PC_MONITOR_API_BASE || 'http://localhost:8080';

  // Override fetch to prepend the API base for relative /api/ paths
  const originalFetch = window.fetch;
  window.fetch = function (input, init) {
    if (typeof input === 'string' && input.startsWith('/api/')) {
      input = API_BASE + input;
    }
    return originalFetch.call(this, input, init);
  };
})();
