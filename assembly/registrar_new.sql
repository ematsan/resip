-- phpMyAdmin SQL Dump
-- version 4.6.6deb5
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Dec 11, 2018 at 10:03 AM
-- Server version: 5.7.24-0ubuntu0.18.04.1
-- PHP Version: 7.2.10-0ubuntu0.18.04.1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `registrar`
--

-- --------------------------------------------------------

--
-- Table structure for table `tauthorization`
--

CREATE TABLE `tauthorization` (
  `fidauth` int(11) NOT NULL,
  `fidudfk` int(11) NOT NULL,
  `fpassword` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `tdomain`
--

CREATE TABLE `tdomain` (
  `fiddomain` int(11) NOT NULL,
  `fdomain` varchar(255) NOT NULL,
  `fidrealm` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `tforward`
--

CREATE TABLE `tforward` (
  `fidforward` int(11) NOT NULL,
  `fiddomainfk` int(11) NOT NULL,
  `fidprotocolfk` int(11) NOT NULL,
  `fport` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `tprotocol`
--

CREATE TABLE `tprotocol` (
  `fidprotocol` int(11) NOT NULL,
  `fprotocol` varchar(20) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `tregistrar`
--

CREATE TABLE `tregistrar` (
  `fidreg` int(11) NOT NULL,
  `fidudfk` int(11) NOT NULL,
  `fcallid` varchar(255) NOT NULL,
  `fiduserfk` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `troute`
--

CREATE TABLE `troute` (
  `fidroute` int(11) NOT NULL,
  `fidregfk` int(11) NOT NULL,
  `fidforwardfk` int(11) NOT NULL,
  `ftime` datetime NOT NULL,
  `fexpires` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `tuser`
--

CREATE TABLE `tuser` (
  `fiduser` int(11) NOT NULL,
  `fname` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `tuserdomain`
--

CREATE TABLE `tuserdomain` (
  `fidud` int(11) NOT NULL,
  `fiduserfk` int(11) NOT NULL,
  `fiddomainfk` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `tauthorization`
--
ALTER TABLE `tauthorization`
  ADD PRIMARY KEY (`fidauth`),
  ADD KEY `fidud` (`fidudfk`);

--
-- Indexes for table `tdomain`
--
ALTER TABLE `tdomain`
  ADD PRIMARY KEY (`fiddomain`),
  ADD UNIQUE KEY `fdomain` (`fdomain`),
  ADD KEY `fidrealm` (`fidrealm`);

--
-- Indexes for table `tforward`
--
ALTER TABLE `tforward`
  ADD PRIMARY KEY (`fidforward`),
  ADD KEY `fidprotocol` (`fidprotocolfk`),
  ADD KEY `fiddomain` (`fiddomainfk`);

--
-- Indexes for table `tprotocol`
--
ALTER TABLE `tprotocol`
  ADD PRIMARY KEY (`fidprotocol`);

--
-- Indexes for table `tregistrar`
--
ALTER TABLE `tregistrar`
  ADD PRIMARY KEY (`fidreg`),
  ADD KEY `fiduser` (`fidudfk`),
  ADD KEY `fidmain` (`fiduserfk`);

--
-- Indexes for table `troute`
--
ALTER TABLE `troute`
  ADD PRIMARY KEY (`fidroute`),
  ADD KEY `fidreg` (`fidregfk`),
  ADD KEY `fidforward` (`fidforwardfk`);

--
-- Indexes for table `tuser`
--
ALTER TABLE `tuser`
  ADD PRIMARY KEY (`fiduser`);

--
-- Indexes for table `tuserdomain`
--
ALTER TABLE `tuserdomain`
  ADD PRIMARY KEY (`fidud`),
  ADD KEY `fiddomainfk` (`fiddomainfk`),
  ADD KEY `fiduserfk` (`fiduserfk`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `tauthorization`
--
ALTER TABLE `tauthorization`
  MODIFY `fidauth` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;
--
-- AUTO_INCREMENT for table `tdomain`
--
ALTER TABLE `tdomain`
  MODIFY `fiddomain` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;
--
-- AUTO_INCREMENT for table `tforward`
--
ALTER TABLE `tforward`
  MODIFY `fidforward` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;
--
-- AUTO_INCREMENT for table `tprotocol`
--
ALTER TABLE `tprotocol`
  MODIFY `fidprotocol` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;
--
-- AUTO_INCREMENT for table `tregistrar`
--
ALTER TABLE `tregistrar`
  MODIFY `fidreg` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;
--
-- AUTO_INCREMENT for table `troute`
--
ALTER TABLE `troute`
  MODIFY `fidroute` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;
--
-- AUTO_INCREMENT for table `tuser`
--
ALTER TABLE `tuser`
  MODIFY `fiduser` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;
--
-- AUTO_INCREMENT for table `tuserdomain`
--
ALTER TABLE `tuserdomain`
  MODIFY `fidud` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;
--
-- Constraints for dumped tables
--

--
-- Constraints for table `tauthorization`
--
ALTER TABLE `tauthorization`
  ADD CONSTRAINT `tauthorization_ibfk_1` FOREIGN KEY (`fidudfk`) REFERENCES `tuserdomain` (`fidud`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `tdomain`
--
ALTER TABLE `tdomain`
  ADD CONSTRAINT `tdomain_ibfk_1` FOREIGN KEY (`fidrealm`) REFERENCES `tdomain` (`fiddomain`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `tforward`
--
ALTER TABLE `tforward`
  ADD CONSTRAINT `tforward_ibfk_1` FOREIGN KEY (`fidprotocolfk`) REFERENCES `tprotocol` (`fidprotocol`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tforward_ibfk_2` FOREIGN KEY (`fiddomainfk`) REFERENCES `tdomain` (`fiddomain`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `tregistrar`
--
ALTER TABLE `tregistrar`
  ADD CONSTRAINT `tregistrar_ibfk_1` FOREIGN KEY (`fiduserfk`) REFERENCES `tuser` (`fiduser`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tregistrar_ibfk_2` FOREIGN KEY (`fidudfk`) REFERENCES `tuserdomain` (`fidud`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `troute`
--
ALTER TABLE `troute`
  ADD CONSTRAINT `troute_ibfk_1` FOREIGN KEY (`fidregfk`) REFERENCES `tregistrar` (`fidreg`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `troute_ibfk_2` FOREIGN KEY (`fidforwardfk`) REFERENCES `tforward` (`fidforward`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `tuserdomain`
--
ALTER TABLE `tuserdomain`
  ADD CONSTRAINT `tuserdomain_ibfk_1` FOREIGN KEY (`fiddomainfk`) REFERENCES `tdomain` (`fiddomain`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `tuserdomain_ibfk_2` FOREIGN KEY (`fiduserfk`) REFERENCES `tuser` (`fiduser`) ON DELETE CASCADE ON UPDATE CASCADE;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
