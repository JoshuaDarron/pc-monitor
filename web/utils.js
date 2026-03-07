/**
 * Format a value in MB, converting to GB if >= 1024 MB.
 * @param {number} bytes - Value in MB
 * @returns {string} Formatted string like "512 MB" or "1.5 GB"
 */
function formatBytes(bytes) {
    if (bytes >= 1024) {
        return (bytes / 1024).toFixed(1) + ' GB';
    }
    return bytes + ' MB';
}

/**
 * Return a performance class based on a value and threshold map.
 * @param {number} value
 * @param {{excellent: number, good: number, fair: number}} thresholds
 * @returns {'excellent'|'good'|'fair'|'poor'}
 */
function getPerformanceClass(value, thresholds) {
    if (value >= thresholds.excellent) return 'excellent';
    if (value >= thresholds.good) return 'good';
    if (value >= thresholds.fair) return 'fair';
    return 'poor';
}

/**
 * Push a value onto a history array, keeping at most maxSize entries.
 * @param {Array} arr - History array (mutated in place)
 * @param {*} value - Value to append
 * @param {number} [maxSize=60] - Maximum buffer length
 */
function pushHistory(arr, value, maxSize) {
    if (maxSize === undefined) maxSize = 60;
    arr.push(value);
    if (arr.length > maxSize) arr.shift();
}

// Export for testing (Node/Vitest), no-op in browser
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { formatBytes, getPerformanceClass, pushHistory };
}
