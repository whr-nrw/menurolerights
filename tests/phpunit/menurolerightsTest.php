<?php
/** 
 * menurolerights module tests
 * 
 * Contains all PHPUnit tests for this module
 *
 * @author Wolfgang Hauertmann <info@hauertmann.com>
 * @copyright (c) 2013 Wolfgang Hauertmann
 * 
 */

/** 
 * Embeds tests into Drupal framework
 */
define('DRUPAL_ROOT', getcwd());
require_once DRUPAL_ROOT . '/includes/bootstrap.inc';
$_SERVER['REQUEST_METHOD'] = 'get';
$_SERVER['REMOTE_ADDR'] = '127.0.0.1';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

/**
 * Class implements all test for this module
 * 
 */
class menurolerightsTest extends PHPUnit_Framework_TestCase {
    
/**
 * Implements test_disableureg_menu_alter();
 *
 * @return void 
 * 
 */  
  public function test_menurolerights_form_alter() {
    $form_id = 'menu_overview_form';
    $form_state = null; // not used
    $form = array(
        // 'mlid:679' => ...
    );
    menurolerights_form_alter($form, $form_state, $form_id);
    $this->assertEquals($form, array());
  }
  
}





