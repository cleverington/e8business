<?php
/**
 * @file cache.inc
 *
 * Functions to load Views' data so that it knows what is available to
 * build queries from.
 */

/**
 * Fetch Views' data from the cache
 *
 * @param $move
 *   Under certain circumstances it makes sense to not get the moved table, but the old one.
 *   One example is views_get_handler.
 */
function _views_fetch_data($table = NULL, $move = TRUE, $reset = FALSE) {
  static $cache = NULL;
  static $recursion_protection = array();
  if (!isset($cache) || $reset) {
    $start = microtime(TRUE);
    // NOTE: This happens whether we retrieve them from cache or otherwise.

    $data = views_cache_get('views_data', TRUE);
    if (!empty($data->data)) {
      $cache = $data->data;
    }

    if (empty($cache)) {
      views_include_handlers();
      $cache = module_invoke_all('views_data');
      foreach (module_implements('views_data_alter') as $module) {
        $function = $module . '_views_data_alter';
        $function($cache);
      }

      views_cache_set('views_data', $cache, TRUE);
    }
  }

  if (!$table) {
    return $cache;
  }
  if (isset($cache[$table])) {
    // Support old views_data entries conversion.
    if (isset($cache[$table]['moved to']) && $move) {
      $moved_table = $cache[$table]['moved to'];
      if (!empty($recursion_protection[$table])) {
        // recursion detected!
        return NULL;
      }
      $recursion_protection[$table] = TRUE;
      $data = _views_fetch_data($moved_table);
      $recursion_protection = array();
      return $data;
    }
    else {
      return $cache[$table];
    }
  }

  // Return an empty array if there is no match.
  return array();
}

/**
 * Fetch the plugin data from cache.
 */
function _views_fetch_plugin_data($type = NULL, $plugin = NULL, $reset = FALSE) {
  static $cache = NULL;
  if (!isset($cache) || $reset) {
    $start = microtime(TRUE);
    views_include('plugins');
    views_include_handlers();

    $cache = views_discover_plugins();

  }

  if (!$type && !$plugin) {
    return $cache;
  }
  elseif (!$plugin) {
    // Not in the if above so the else below won't run
    if (isset($cache[$type])) {
      return $cache[$type];
    }
  }
  elseif (isset($cache[$type][$plugin])) {
    return $cache[$type][$plugin];
  }

  // Return an empty array if there is no match.
  return array();
}

/**
 * Set a cached item in the views cache.
 *
 * This is just a convenience wrapper around cache_set().
 *
 * @param $cid
 *   The cache ID of the data to store.
 * @param $data
 *   The data to store in the cache. Complex data types will be automatically serialized before insertion.
 *   Strings will be stored as plain text and not serialized.
 * @param $use_language
 *   If TRUE, the data will be cached specific to the currently active language.
 */
function views_cache_set($cid, $data, $use_language = FALSE) {
  global $language;

  if (variable_get('views_skip_cache', FALSE)) {
    return;
  }
  if ($use_language) {
    $cid .= ':' . $language->language;
  }

  cache_set($cid, $data, 'cache_views');
}

/**
 * Return data from the persistent views cache.
 *
 * This is just a convenience wrapper around cache_get().
 *
 * @param $cid
 *   The cache ID of the data to retrieve.
 * @param $use_language
 *   If TRUE, the data will be requested specific to the currently active language.
 */
function views_cache_get($cid, $use_language = FALSE) {
  global $language;

  if (variable_get('views_skip_cache', FALSE)) {
    return 0;
  }
  if ($use_language) {
    $cid .= ':' . $language->language;
  }

  return cache_get($cid, 'cache_views');
}
