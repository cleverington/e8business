<?php

/**
 * Entrypoint for drush rsync.
 *
 * @param source
 *   A site alias ("@dev") or site specification ("/path/to/drupal#mysite.com")
 *   followed by an optional path (":path/to/sync"), or any path
 *   that could be passed to rsync ("user@server.com:/path/to/dir/").
 * @param destination
 *   Same format as source.
 * @param additional_options
 *   An array of options that overrides whatever was passed in on
 *   the command line (like the 'process' context, but only for
 *   the scope of this one call).
 */
function drush_core_rsync($source, $destination, $additional_options = array()) {
  // Preflight destination in case it defines aliases used by the source
  _drush_sitealias_preflight_path($destination);
  // After preflight, evaluate file paths
  $source_settings = drush_sitealias_evaluate_path($source, $additional_options);
  $destination_settings = drush_sitealias_evaluate_path($destination, $additional_options);
  $source_path = $source_settings['evaluated-path'];
  $destination_path = $destination_settings['evaluated-path'];

  if (!isset($source_settings)) {
    return drush_set_error('DRUSH_BAD_PATH', dt('Could not evaluate source path !path.', array('!path' => $source)));
  }
  if (!isset($destination_settings)) {
    return drush_set_error('DRUSH_BAD_PATH', dt('Could not evaluate destination path !path.', array('!path' => $destination)));
  }
    
  // Check to see if this is an rsync multiple command (multiple sources and multiple destinations)
  $is_multiple = drush_do_multiple_command('rsync', $source_settings, $destination_settings, TRUE);
  
  if ($is_multiple === FALSE) {
    // If the user path is the same for the source and the destination, then
    // always add a slash to the end of the source.  If the user path is not
    // the same in the source and the destination, then you need to know how
    // rsync paths work, and put on the trailing '/' if you want it.
    if ($source_settings['user-path'] == $destination_settings['user-path']) {
      $source_path .= '/';
    }
    // Prompt for confirmation. This is destructive.
    if (!drush_get_context('DRUSH_SIMULATE')) {
      drush_print(dt("You will destroy data from !target and replace with data from !source", array('!source' => $source_path, '!target' => $destination_path)));
      if (!drush_confirm(dt('Do you really want to continue?'))) {
        // was: return drush_set_error('CORE_SYNC_ABORT', 'Aborting.');
        return drush_user_abort();
      }
    }

    // Exclude settings is the default only when both the source and
    // the destination are aliases or site names.  Therefore, include
    // settings will be the default whenever either the source or the
    // destination contains a : or a /.
    $include_settings_is_default = (strpos($source . $destination, ':') !== FALSE) || (strpos($source . $destination, '/') !== FALSE);

    // Go ahead and call rsync with the paths we determined
    return drush_core_call_rsync($source_path, $destination_path, $additional_options, $include_settings_is_default);
  }
}

/**
 * Make a direct call to rsync after the source and destination paths
 * have been evaluated.
 *
 * @param $source
 *   Any path that can be passed to rsync.
 * @param $destination
 *   Any path that can be passed to rsync.
 * @param $additional_options
 *   An array of options that overrides whatever was passed in on the command
 *   line (like the 'process' context, but only for the scope of this one
 *   call).
 * @param $include_settings_is_default
 *   If TRUE, then settings.php will be transferred as part of the rsync unless
 *   --exclude-conf is specified.  If FALSE, then settings.php will be excluded
 *   from the transfer unless --include-conf is specified.
 * @param $live_output
 *   If TRUE, output goes directly to the terminal using system(). If FALSE,
 *   rsync is executed with drush_shell_exec() with output in
 *   drush_shell_exec_output().
 *
 * @return
 *   TRUE on success, FALSE on failure.
 */
function drush_core_call_rsync($source, $destination, $additional_options = array(), $include_settings_is_default = TRUE, $live_output = TRUE) {
  // Exclude vcs reserved files.
  $options = '';
  if (!_drush_rsync_option_exists('include-vcs', $additional_options)) {
    $vcs_files = drush_version_control_reserved_files();
    foreach ($vcs_files as $file) {
      $options .= ' --exclude="'.$file.'"';
    }
  }

  $mode = '-az';
  // Process --include-path and --exclude-path options the same way
  foreach (array('include', 'exclude') as $include_exclude) {
    // Get the option --include-path or --exclude-path and explode to an array of paths
    // that we will translate into an --include or --exclude option to pass to rsync
    $inc_ex_path = explode(PATH_SEPARATOR, drush_get_option(array($include_exclude . '-path', $include_exclude . '-paths'), ''));
    foreach ($inc_ex_path as $one_path_to_inc_ex) {
      if (!empty($one_path_to_inc_ex)) {
        $options .= ' --' . $include_exclude . '="' . $one_path_to_inc_ex . '"';
      }
    }
  }
  // drush_core_rsync passes in $include_settings_is_default such that
  // 'exclude-conf' is the default when syncing from one alias to
  // another, and 'include-conf' is the default when a path component
  // is included.
  if ($include_settings_is_default ? _drush_rsync_option_exists('exclude-conf', $additional_options) : !_drush_rsync_option_exists('include-conf', $additional_options)) {
    $options .= ' --exclude="settings.php"';
  }
  if (_drush_rsync_option_exists('exclude-sites', $additional_options)) {
    $options .= ' --include="sites/all" --exclude="sites/*"';
  }
  if (_drush_rsync_option_exists('mode', $additional_options)) {
    $mode = "-" . drush_get_option_override($additional_options, 'mode');
  }
  if (drush_get_context('DRUSH_VERBOSE')) {
    // the drush_op() will be verbose about the command that gets executed.
    $mode .= 'v';
    $options .= ' --stats --progress';
  }
  $rsync_available_options = array(
    // unary options
    'archive', // -a
    'recursive', // -r
    'relative', // -R
    'backup', // -b
    'update', // -u
    'checksum', // -c
    'dirs', // -d
    'links', // -l
    'copy-links', // -L
    'copy-dirlinks', // -k
    'keep-dirlinks', // -K
    'hard-links', // -H
    'perms', // -p
    'executability', // -E
    'acls', // -A
    'xattrs', // -X
    'owner', // -o
    'group', // -g
    'times', // -t
    'omit-dir-times', // -O
    'sparse', // -S
    'dry-run', // -n
    'whole-file', // -W
    'one-file-system', // -x
    'prune-empty-dirs', // -m
    'ignore-times', // -I
    'fuzzy', // -y
    'cvs-exclude', // -C
    'compress', // -Z
    'protect-args', // -s
    '8-bit-output', // -8
    'human-readable', // -h
    'itemize-changes', // -i
    'copy-unsafe-links',
    'safe-links',
    'no-implied-dirs',
    'inplace',
    'append',
    'append-verify',
    'existing',
    'remove-source-files',
    'delete',
    'delete-before',
    'delete-during',
    'delete-delay',
    'delete-after',
    'delete-excluded',
    'ignore-errors',
    'force',
    'ignore-existing',
    'partial',
    'delay-updates',
    'numeric-ids',
    'size-only',
    'blocking-io',
    'stats',
    'progress',
    'list-only',
    // options with values
    'block-size',
    'backup-dir',
    'suffix',
    'chmod',
    'rsync-path',
    'modify-window',
    'compare-dest',
    'copy-dest',
    'link-dest',
    'skip-compress',
    'filter',
    'exclude',
    'exclude-from',
    'include',
    'include-from',
    'files-from',
    'address',
    'port',
    'sockopts',
    'out-format',
    'bwlimit',
    'iconv',
    'checksum-seed',
    'max-delete',
    'max-size',
    'min-size',
    'partial-dir',
    'timeout',
    'temp-dir',
    'compress-level',
    'out-format',
    'protocol',
    );
  // Check if the user has set $options['rsync-version'] to enable rsync legacy version support.
  // Drush was written for rsync 2.6.9 or later, so assume that version if nothing was explicitly set.
  $rsync_version = drush_get_option(array('rsync-version','source-rsync-version','target-rsync-version'), '2.6.9');
  foreach ($rsync_available_options as $test_option) {
    $value = drush_get_option_override($additional_options, $test_option);
    // Downgrade some options for older versions of rsync
    if ($test_option == 'remove-source-files') {
      if (version_compare($rsync_version, '2.6.4', '<')) {
        $test_option = NULL;
        drush_log('Rsync does not support --remove-sent-files prior to version 2.6.4; some temporary files may remain undeleted.', 'warning');
      }
      elseif (version_compare($rsync_version, '2.6.9', '<')) {
        $test_option = 'remove-sent-files';
      }
    }
    if ((isset($test_option)) && (isset($value))) {
      if ($value === TRUE) {
        $options .= " --$test_option";
      }
      else {
        $options .= " --$test_option=" . escapeshellarg($value);
      }
    }
  }

  $ssh_options = drush_get_option_override($additional_options, 'ssh-options', '');
  $exec = "rsync -e 'ssh $ssh_options' $mode$options $source $destination";

  if ($live_output) {
    $exec_result = drush_op_system($exec);
    $result = ($exec_result == 0);
  }
  else {
    $result = drush_shell_exec($exec);
  }
  
  if (!$result) {
    drush_set_error('DRUSH_RSYNC_FAILED', dt("Could not rsync from !source to !dest", array('!source' => $source, '!dest' => $destination)));
  }

  return $result;
}

function _drush_rsync_option_exists($option, $additional_options) {
  if (array_key_exists($option, $additional_options)) {
    return TRUE;
  }
  else {
    return drush_get_option($option, FALSE);
  }
}
