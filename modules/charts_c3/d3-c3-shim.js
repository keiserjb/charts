/**
 * @file
 * D3 v6/v7 compatibility shim for C3.js.
 */

/* global d3 */
(function () {
  if (typeof d3 === 'undefined') {
    return;
  }

  if (!d3.set) {
    d3.set = function (values) {
      var data = new Set(values || []);
      return {
        has: function (value) {
          return data.has(value);
        },
        add: function (value) {
          data.add(value);
          return this;
        },
        remove: function (value) {
          data.delete(value);
          return this;
        },
        clear: function () {
          data.clear();
          return this;
        },
        size: function () {
          return data.size;
        },
        empty: function () {
          return data.size === 0;
        },
        values: function () {
          return Array.from(data);
        }
      };
    };
  }

  if (!d3.mouse) {
    d3.mouse = function (node) {
      var event = d3.event || window.event;
      if (event && d3.pointer) {
        return d3.pointer(event, node);
      }
      return [0, 0];
    };
  }
})();
