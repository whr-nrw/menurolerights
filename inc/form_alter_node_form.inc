<?php
/** 
 * menurolerights module include file  
 * 
 * Form 'node'
 *
 * @author Wolfgang Hauertmann <info@hauertmann.com>
 * @copyright (c) 2014 Wolfgang Hauertmann
 * @license http://www.gnu.org/licenses/gpl-2.0.html GPL-2.0
 * 
 */

/**
 * Modify node form after build
 * 
 * @param array $form (call by ref) 
 *   A structured array containing the elements and properties of the form.
 * @param array $form_state (call by ref) 
 *   An array that stores information about the form's current state
 *   during processing. 
 * @return void 
 */
function _node_form_after_build($form, &$form_state) {  
  global $user; 
  // Get a list of all possible menu parent items of selector element in this node form  
  $options = _hrmrr_getAllOptionsForMenuParentSelectOfNodeForm($form); 
  // Get rules
  $rules = _hrmrr_getListOfActiveRules();  
  // Over all active rules ...
  $foptions = array();
  foreach ($rules as $rule) {
    // If rule matches one role of the user ...
    if (in_array($rule['role'], $user->roles)) { 
      // append options allowed by this rule
      $foptions = array_merge($foptions,_hrmrr_filterOptionsToMenuParentSelectByRule($form, $options, $rule));   
    }
  }
  // if no limiting rule exists don't do a modification
  if (empty($foptions))
    return $form;
  // Fill parent menu item selector element with filtered options
  _hrmrr_buildNewParentMenuSelect($form, $foptions);
  // Return modified form
  return $form;
}

