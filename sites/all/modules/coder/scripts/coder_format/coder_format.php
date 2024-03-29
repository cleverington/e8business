<?php


/**
 * @file
 * Coder format shell invocation script.
 *
 * @param $filepath
 * 	 One or more files or directories containing the source code to process. If
 *   a directory is given, all files in that directory are processed recursively.
 *   Each file will be replaced with the formatted source code. Each original
 *   file is automatically backup to <filename>.coder.orig.
 * @param --undo
 *   If the optional --undo argument is given, processed files are restored from
 *   their backups. Coder format automatically searches for the latest backup
 *   file (*.coder.orig) and each filename (without extension .coder.orig) is
 *   replaced with its backup file.
 *
 * @usage
 *   php coder_format.php <filepath> [filepath] ...
 *   php coder_format.php <path> [path] ...
 *   php coder_format.php --undo <filepath> ...
 *   php coder_format.php --undo <path> ...
 *
 * @example
 *   php coder_format.php modules/node/node.module
 *   php coder_format.php index.php modules/node/node.module
 *   php coder_format.php /home/drupal
 *   php coder_format.php /home/drupal/includes /home/drupal/modules
 *   php coder_format.php --undo modules/node/node.module
 *   php coder_format.php --undo /home/drupal
 *
 * Important note for Windows users:
 * - Ensure to encapsulate filename arguments with double quotes.
 */

// We are on the command line, so output only real errors.
error_reporting(E_ERROR | E_WARNING | E_PARSE);

require_once realpath(dirname($_SERVER['PHP_SELF'])) . '/coder_format.inc';

if (!empty($_SERVER['argv'])) {
  // Remove self-reference.
  array_shift($_SERVER['argv']);

  // Process command-line arguments.
  $files    = array();
  $undo     = FALSE;

  for ($c = 0, $cc = count($_SERVER['argv']); $c < $cc; ++$c) {
    switch ($_SERVER['argv'][$c]) {
      case '--undo':
        $undo = TRUE;
        break;

      default:
        $files[] = $_SERVER['argv'][$c];
        break;
    }
  }

  foreach ($files as $file) {
    // If argument is a directory, process it recursively.
    if (is_dir($file)) {
      coder_format_recursive($file, $undo);
    }
    else {
      coder_format_file($file, $undo);
    }
  }
}

/**
 * @defgroup coder_format_file_functions
 * @{
 * These functions are copied from Drupal's file.inc. Almost all function calls
 * to other Drupal functions have been removed since they would always return
 * FALSE.
 */

define('FILE_CREATE_DIRECTORY', 1);
define('FILE_EXISTS_RENAME', 0);
define('FILE_EXISTS_REPLACE', 1);
define('FILE_EXISTS_ERROR', 2);

/**
 * Check that the directory exists and is writable. Directories need to
 * have execute permissions to be considered a directory by FTP servers, etc.
 *
 * @param $directory A string containing the name of a directory path.
 * @param $mode A Boolean value to indicate if the directory should be created
 *   if it does not exist or made writable if it is read-only.
 * @param $form_item An optional string containing the name of a form item that
 *   any errors will be attached to. This is useful for settings forms that
 *   require the user to specify a writable directory. If it can't be made to
 *   work, a form error will be set preventing them from saving the settings.
 * @return FALSE when directory not found, or TRUE when directory exists.
 */
function file_check_directory(&$directory, $mode = 0, $form_item = NULL) {
  $directory = rtrim($directory, '/\\');

  // Check if directory exists.
  if (!is_dir($directory)) {
    // coder_format does not alter the filesystem. 23/01/2008 sun
    if (!file_exists($directory)) {
      drupal_set_message(t('The directory %directory does not exist.', array('%directory' => $directory)));
      return FALSE;
    }
  }

  // Check to see if the directory is writable.
  if (!is_writable($directory)) {
    // coder_format does not alter the filesystem. 23/01/2008 sun
    drupal_set_message(t('The directory %directory is not writable', array('%directory' => $directory)));
    return FALSE;
  }

  // coder_format is applied outside of Drupal in most cases. 23/01/2008 sun

  return TRUE;
}

/**
 * Checks path to see if it is a directory, or a dir/file.
 *
 * @param $path A string containing a file path. This will be set to the
 *   directory's path.
 * @return If the directory is not in a Drupal writable directory, FALSE is
 *   returned. Otherwise, the base name of the path is returned.
 */
function file_check_path(&$path) {
  // Check if path is a directory.
  // coder_format uses file_check_path() to check for files only. 26/01/2008 sun

  // Check if path is a possible dir/file.
  $filename = basename($path);
  $path = dirname($path);
  if (file_check_directory($path)) {
    return $filename;
  }

  return FALSE;
}

/**
 * Copies a file to a new location. This is a powerful function that in many ways
 * performs like an advanced version of copy().
 * - Checks if $source and $dest are valid and readable/writable.
 * - Performs a file copy if $source is not equal to $dest.
 * - If file already exists in $dest either the call will error out, replace the
 *   file or rename the file based on the $replace parameter.
 *
 * @param $source A string specifying the file location of the original file.
 *   This parameter will contain the resulting destination filename in case of
 *   success.
 * @param $dest A string containing the directory $source should be copied to.
 *   If this value is omitted, Drupal's 'files' directory will be used.
 * @param $replace Replace behavior when the destination file already exists.
 *   - FILE_EXISTS_REPLACE - Replace the existing file
 *   - FILE_EXISTS_RENAME - Append _{incrementing number} until the filename is unique
 *   - FILE_EXISTS_ERROR - Do nothing and return FALSE.
 * @return True for success, FALSE for failure.
 */
function file_copy(&$source, $dest = 0, $replace = FILE_EXISTS_RENAME) {
  // $dest is almost always outside of Drupal. 23/01/2008 sun
  // $dest = file_create_path($dest);

  $directory = $dest;
  $basename = file_check_path($directory);

  // Make sure we at least have a valid directory.
  if ($basename === FALSE) {
    $source = is_object($source) ? $source->filepath : $source;
    drupal_set_message(t('The selected file %file could not be uploaded, because the destination %directory is not properly configured.', array('%file' => $source, '%directory' => $dest)), 'error');
    return 0;
  }

  // Removed upload handling. coder_format does not deal with uploads. 23/01/2008 sun

  $source = realpath($source);
  if (!file_exists($source)) {
    drupal_set_message(t('The selected file %file could not be copied, because no file by that name exists. Please check that you supplied the correct filename.', array('%file' => $source)), 'error');
    return 0;
  }

  // If the destination file is not specified then use the filename of the source file.
  $basename = $basename ? $basename : basename($source);
  $dest = $directory . '/' . $basename;

  // Make sure source and destination filenames are not the same, makes no sense
  // to copy it if they are. In fact copying the file will most likely result in
  // a 0 byte file. Which is bad. Real bad.
  if ($source != realpath($dest)) {
    if (!$dest = file_destination($dest, $replace)) {
      drupal_set_message(t('The selected file %file could not be copied, because a file by that name already exists in the destination.', array('%file' => $source)), 'error');
      return FALSE;
    }

    if (!@copy($source, $dest)) {
      drupal_set_message(t('The selected file %file could not be copied.', array('%file' => $source)), 'error');
      return 0;
    }

    // Give everyone read access so that FTP'd users or
    // non-webserver users can see/read these files,
    // and give group write permissions so group members
    // can alter files uploaded by the webserver.
    @chmod($dest, 0664);
  }

  // Removed upload handling. coder_format does not deal with uploads. 23/01/2008 sun
  $source = $dest;

  return 1; // Everything went ok.
}

/**
 * Determines the destination path for a file depending on how replacement of
 * existing files should be handled.
 *
 * @param $destination A string specifying the desired path.
 * @param $replace Replace behavior when the destination file already exists.
 *   - FILE_EXISTS_REPLACE - Replace the existing file
 *   - FILE_EXISTS_RENAME - Append _{incrementing number} until the filename is
 *     unique
 *   - FILE_EXISTS_ERROR - Do nothing and return FALSE.
 * @return The destination file path or FALSE if the file already exists and
 *   FILE_EXISTS_ERROR was specified.
 */
function file_destination($destination, $replace) {
  if (file_exists($destination)) {
    switch ($replace) {
      case FILE_EXISTS_RENAME:
        $basename = basename($destination);
        $directory = dirname($destination);
        $destination = file_create_filename($basename, $directory);
        break;

      case FILE_EXISTS_ERROR:
        drupal_set_message(t('The selected file %file could not be copied, because a file by that name already exists in the destination.', array('%file' => $destination)), 'error');
        return FALSE;
    }
  }
  return $destination;
}

/**
 * Moves a file to a new location.
 * - Checks if $source and $dest are valid and readable/writable.
 * - Performs a file move if $source is not equal to $dest.
 * - If file already exists in $dest either the call will error out, replace the
 *   file or rename the file based on the $replace parameter.
 *
 * @param $source A string specifying the file location of the original file.
 *   This parameter will contain the resulting destination filename in case of
 *   success.
 * @param $dest A string containing the directory $source should be copied to.
 *   If this value is omitted, Drupal's 'files' directory will be used.
 * @param $replace Replace behavior when the destination file already exists.
 *   - FILE_EXISTS_REPLACE - Replace the existing file
 *   - FILE_EXISTS_RENAME - Append _{incrementing number} until the filename is unique
 *   - FILE_EXISTS_ERROR - Do nothing and return FALSE.
 * @return True for success, FALSE for failure.
 */
function file_move(&$source, $dest = 0, $replace = FILE_EXISTS_RENAME) {
  $path_original = is_object($source) ? $source->filepath : $source;

  if (file_copy($source, $dest, $replace)) {
    $path_current = is_object($source) ? $source->filepath : $source;

    if ($path_original == $path_current || file_delete($path_original)) {
      return 1;
    }
    drupal_set_message(t('The removal of the original file %file has failed.', array('%file' => $path_original)), 'error');
  }
  return 0;
}

/**
 * Create a full file path from a directory and filename. If a file with the
 * specified name already exists, an alternative will be used.
 *
 * @param $basename string filename
 * @param $directory string directory
 * @return
 */
function file_create_filename($basename, $directory) {
  $dest = $directory . '/' . $basename;

  if (file_exists($dest)) {
    // Destination file already exists, generate an alternative.
    // Always append '.coder.orig' (allows multiple undos). 23/01/2008 sun
    $name = $basename;

    $counter = 0;
    do {
      $dest = $directory . '/' . $name . str_repeat('.coder.orig', $counter++);
    } while (file_exists($dest));
  }

  return $dest;
}

/**
 * Delete a file.
 *
 * @param $path A string containing a file path.
 * @return TRUE for success, FALSE for failure.
 */
function file_delete($path) {
  if (is_file($path)) {
    return unlink($path);
  }
}

/**
 * Finds all files that match a given mask in a given directory.
 * Directories and files beginning with a period are excluded; this
 * prevents hidden files and directories (such as SVN working directories)
 * from being scanned.
 *
 * @param $dir
 *   The base directory for the scan, without trailing slash.
 * @param $mask
 *   The regular expression of the files to find.
 * @param $nomask
 *   An array of files/directories to ignore.
 * @param $callback
 *   The callback function to call for each match.
 * @param $recurse
 *   When TRUE, the directory scan will recurse the entire tree
 *   starting at the provided directory.
 * @param $key
 *   The key to be used for the returned array of files. Possible
 *   values are "filename", for the path starting with $dir,
 *   "basename", for the basename of the file, and "name" for the name
 *   of the file without an extension.
 * @param $min_depth
 *   Minimum depth of directories to return files from.
 * @param $depth
 *   Current depth of recursion. This parameter is only used internally and should not be passed.
 *
 * @return
 *   An associative array (keyed on the provided key) of objects with
 *   "path", "basename", and "name" members corresponding to the
 *   matching files.
 */
function file_scan_directory($dir, $mask, $nomask = array('.', '..', 'CVS', '.svn', '.git'), $callback = 0, $recurse = TRUE, $key = 'filename', $min_depth = 0, $depth = 0) {
  $key = (in_array($key, array('filename', 'basename', 'name')) ? $key : 'filename');
  $files = array();

  if (is_dir($dir) && $handle = opendir($dir)) {
    while ($file = readdir($handle)) {
      if (!in_array($file, $nomask) && $file[0] != '.') {
        if (is_dir("$dir/$file") && $recurse) {
          // Give priority to files in this folder by merging them in after any subdirectory files.
          $files = array_merge(file_scan_directory("$dir/$file", $mask, $nomask, $callback, $recurse, $key, $min_depth, $depth + 1), $files);
        }
        elseif ($depth >= $min_depth && ereg($mask, $file)) {
          // Always use this match over anything already set in $files with the same $$key.
          $filename = "$dir/$file";
          $basename = basename($file);
          $name = substr($basename, 0, strrpos($basename, '.'));
          $files[$$key] = new stdClass();
          $files[$$key]->filename = $filename;
          $files[$$key]->basename = $basename;
          $files[$$key]->name = $name;
          if ($callback) {
            $callback($filename);
          }
        }
      }
    }

    closedir($handle);
  }

  return $files;
}

/**
 * @} End of "defgroup coder_format_file_functions".
 */

/**
 * @defgroup coder_format_stub_functions
 * @{
 */
function t($string, $args = 0) {
  if (!$args) {
    return $string;
  }
  else {
    return strtr($string, $args);
  }
}

function drupal_set_message($message = NULL, $type = 'status') {
  if ($type == 'error') {
    echo str_repeat('-', 80);
    echo "\nERROR: $message\n";
    echo str_repeat('-', 80);
    echo "\n";
  }
  else {
    echo "$message\n";
  }
}

/**
 * @} End of "defgroup coder_format_stub_functions".
 */

