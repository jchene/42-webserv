<?php
$request_method = $_SERVER['REQUEST_METHOD'];

if ($request_method === 'POST') {
    $firstname = $_POST['firstname'];
    $lastname = $_POST['lastname'];
} elseif ($request_method === 'GET') {
    $firstname = $_GET['firstname'];
    $lastname = $_GET['lastname'];
} else {
    $firstname = $_GET['default'];
    $lastname = $_GET['name'];
}

echo "$firstname $lastname est un(e) super correcteur(rice)! :3";
?>
