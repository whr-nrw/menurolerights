<?php
/** 
 * menurolerights module include file  
 * 
 * Helper functions for alter node form
 *
 * @author Wolfgang Hauertmann <info@hauertmann.com>
 * @copyright (c) 2014 Wolfgang Hauertmann
 * @license http://www.gnu.org/licenses/gpl-2.0.html GPL-2.0
 * 
 */


/**
 * Get all options for menu parent item selector in a node form
 * 
 * @param array $form 
 *   A structured array containing the elements and properties of the form.
 * @return array
 *   All possible options of menu parent select for this node form 
 */
function _hrmrr_getAllOptionsForMenuParentSelectOfNodeForm($form) {
  // $link = $form['#node']->menu; // Generate a list of possible parents 
                                   // (not including this link or descendants).
  $link = NULL; // Generate a list of all menu items  
  $type = $form['#node']->type;
  $options = menu_parent_options(menu_get_menus(), $link['mlid'] ? $link : $type, $type);
  return $options;
}

/**
 * Filter options for menu parent item selector of a node form by rule
 * 
 * @param array $form 
 *   A structured array containing the elements and properties of the form.
 * @param array $options 
 *   An array describing options for the parent menu item selector
 * @param array $rule 
 *   An array describing a rule for accessing a menu area
 * @return array 
 *   By rule filterd options of menu parent item selector for this node form 
 */
function _hrmrr_filterOptionsToMenuParentSelectByRule($form, $options, $rule) {
  // on irregular input return with defined result
  if (empty($options)) {
    return array();
  }
  if (empty($rule)) {
    return $options;
  }
  // Collect menu segments allowed by the rule
  $foptions = array();
  $lev = -1; $slev = -1;
  // Over all menu options ...
  foreach ($options as $key=>$val) {
    // get the option index of the rule
    $anOpt = "$rule[menu_name]:$rule[master_mlid]"; // e.g.: 'main-menu:679'
    // If one of the menu options is the rule option ...
    if ($key == $anOpt) {
      // get level of most top parent option for start level 
      $slev = _hrmrr_getOptLevel($val);
    }
    // Get running level of menu options
    $lev = _hrmrr_getOptLevel($val);
    // If collecting has started ...
    if ($slev > -1) {
      // as long as running level is greater than or equal start level collect menu items
      if ($lev >= $slev) {
        // don't collect next option on same level like start level
        if ($lev == $slev) {
           $slev++;
        }
        // collect a sub level option
        $foptions[$key] = $val; // e.g.: '---Recipes' => level 3
      }
      else {
        break; // no more sub options to collect
      }
    }
  }
  return $foptions;
}


/**
 * Get level of an option
 * 
 * @param string $optVal 
 *   A string like  '---Recipes' which means 'recipes stay on level 3'
 */
function _hrmrr_getOptLevel($optVal) {
  if ($optVal == '')
    return -1;
  $lev = 0;
  $chars = str_split($optVal);
  foreach ($chars as $S) {
    if ($S == '-') {
      $lev++;
    }
    else {
      break;
    }
  }
  return $lev;
}

/**
 * Build a new parent menu item selector by given options
 * 
 * @param array $form (call by ref) 
 *   A structured array containing the elements and properties of the form.
 * @param array $options 
 *   An array describing options for the parent menu item selector
 * @return void 
 */
function _hrmrr_buildNewParentMenuSelect(&$form, $options){
  // Generate a list of possible parents (not including this link or descendants).
  $link = $form['#node']->menu;  
  $type = $form['#node']->type;
   // get default parent menu item
  $default = ($link['mlid'] ? $link['menu_name'] . ':' . $link['plid'] : variable_get('menu_parent_' . $type, 'main-menu:0'));
  // If the current parent menu item is not present in options, use the first
  // available option as default value.
  if (!isset($options[$default])) {
    $array = array_keys($options);
    $default = reset($array);
  }
  // Override options for the parent menu link selection  
  $form['menu']['link']['parent'] = array(
    '#type' => 'select',
    '#title' => t('Parent item'),
    '#default_value' => $default,
    '#options' => $options,
    '#attributes' => array('class' => array('menu-parent-select')),
  );
}

