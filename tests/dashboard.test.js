import { describe, it, expect } from 'vitest';
import { formatBytes, getPerformanceClass, pushHistory } from '../web/utils.js';

describe('formatBytes', () => {
    it('returns MB for values below 1024', () => {
        expect(formatBytes(512)).toBe('512 MB');
    });

    it('returns MB for zero', () => {
        expect(formatBytes(0)).toBe('0 MB');
    });

    it('converts to GB at exactly 1024', () => {
        expect(formatBytes(1024)).toBe('1.0 GB');
    });

    it('converts to GB above 1024', () => {
        expect(formatBytes(2048)).toBe('2.0 GB');
    });

    it('formats GB with one decimal', () => {
        expect(formatBytes(1536)).toBe('1.5 GB');
    });

    it('handles large values', () => {
        expect(formatBytes(32768)).toBe('32.0 GB');
    });
});

describe('getPerformanceClass', () => {
    const thresholds = { excellent: 90, good: 70, fair: 50 };

    it('returns excellent when at threshold', () => {
        expect(getPerformanceClass(90, thresholds)).toBe('excellent');
    });

    it('returns excellent when above threshold', () => {
        expect(getPerformanceClass(100, thresholds)).toBe('excellent');
    });

    it('returns good when at good threshold', () => {
        expect(getPerformanceClass(70, thresholds)).toBe('good');
    });

    it('returns good when between good and excellent', () => {
        expect(getPerformanceClass(85, thresholds)).toBe('good');
    });

    it('returns fair when at fair threshold', () => {
        expect(getPerformanceClass(50, thresholds)).toBe('fair');
    });

    it('returns poor when below fair', () => {
        expect(getPerformanceClass(30, thresholds)).toBe('poor');
    });

    it('returns poor for zero', () => {
        expect(getPerformanceClass(0, thresholds)).toBe('poor');
    });
});

describe('pushHistory', () => {
    it('adds a value to the array', () => {
        const arr = [1, 2, 3];
        pushHistory(arr, 4);
        expect(arr).toEqual([1, 2, 3, 4]);
    });

    it('caps array at 60 entries', () => {
        const arr = Array.from({ length: 60 }, (_, i) => i);
        pushHistory(arr, 999);
        expect(arr.length).toBe(60);
        expect(arr[arr.length - 1]).toBe(999);
        expect(arr[0]).toBe(1); // first element shifted out
    });

    it('does not shift when under 60 entries', () => {
        const arr = [10, 20];
        pushHistory(arr, 30);
        expect(arr).toEqual([10, 20, 30]);
    });

    it('works on empty array', () => {
        const arr = [];
        pushHistory(arr, 42);
        expect(arr).toEqual([42]);
    });
});
