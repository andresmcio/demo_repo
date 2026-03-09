pipeline {
    agent any

    parameters {
        string(name: 'SCANOSS_API_TOKEN_ID', defaultValue:"scanoss-token", description: 'The reference ID for the SCANOSS API TOKEN credential')
        string(name: 'SCANOSS_CLI_DOCKER_IMAGE', defaultValue:"ghcr.io/scanoss/scanoss-py-jenkins:v1.46.0", description: 'SCANOSS CLI Docker Image')
        string(name: 'SCANOSS_API_URL', defaultValue:"https://api.osskb.org/scan/direct", description: 'SCANOSS API URL (optional - default: https://api.osskb.org/scan/direct)')

        string(name: 'SCAN_PATH', defaultValue: '.', description: 'Relative path within the repository to scan (e.g., "src" or "packages/api"). Must be relative, no parent directory references (..) allowed.')

        booleanParam(name: 'SKIP_SNIPPET', defaultValue: false, description: 'Skip the generation of snippets.')
        booleanParam(name: 'SCANOSS_SETTINGS', defaultValue: true, description: 'Settings file to use for scanning.')
        string(name: 'SETTINGS_FILE_PATH', defaultValue: 'scanoss.json', description: 'SCANOSS settings file path.')

        // Dependencies
        booleanParam(name: 'DEPENDENCY_ENABLED', defaultValue: false, description: 'Scan dependencies (optional - default false).')
        string(name: 'DEPENDENCY_SCOPE', defaultValue: '', description: 'Gets development or production dependencies (scopes - prod|dev)')
        string(name: 'DEPENDENCY_SCOPE_INCLUDE', defaultValue: '', description: 'Custom list of dependency scopes to be included. Provide scopes as a comma-separated list.')
        string(name: 'DEPENDENCY_SCOPE_EXCLUDE', defaultValue: '', description: 'Custom list of dependency scopes to be excluded. Provide scopes as a comma-separated list.')

        // Copyleft licenses
        string(name: 'LICENSES_COPYLEFT_INCLUDE', defaultValue: '', description: 'List of Copyleft licenses to append to the default list. Provide licenses as a comma-separated list.')
        string(name: 'LICENSES_COPYLEFT_EXCLUDE', defaultValue: '', description: 'List of Copyleft licenses to remove from default list. Provide licenses as a comma-separated list.')
        string(name: 'LICENSES_COPYLEFT_EXPLICIT', defaultValue: '', description: 'Explicit list of Copyleft licenses to consider. Provide licenses as a comma-separated list.')

        // Jira
        string(name: 'JIRA_CREDENTIALS', defaultValue:"jira-credentials" , description: 'Jira credentials')
        string(name: 'JIRA_URL', defaultValue:"" , description: 'Jira URL')
        string(name: 'JIRA_PROJECT_KEY', defaultValue:"" , description: 'Jira Project Key')
        booleanParam(name: 'CREATE_JIRA_ISSUE', defaultValue: false, description: 'Enable Jira reporting')

        // Policies setup
        booleanParam(name: 'ABORT_ON_POLICY_FAILURE', defaultValue: false, description: 'Abort Pipeline on pipeline Failure')

        // Debug
        booleanParam(name: 'DEBUG', defaultValue: false , description: 'Enable debugging')
    }

    environment {
        // Artifact file names
        SCANOSS_COPYLEFT_REPORT_MD = "scanoss-copyleft-report.md"
        SCANOSS_UNDECLARED_REPORT_MD = "scanoss-undeclared-report.md"
        SCANOSS_RESULTS_OUTPUT_FILE_NAME = "results.json"
        SCANOSS_CYCLONEDX_REPORT_FILE_NAME = "scanoss-cyclonedx.json"
        SCANOSS_SPDX_REPORT_FILE_NAME = "scanoss-spdx.json"
        SCANOSS_CSV_REPORT_FILE_NAME = "scanoss-sbom.csv"
        SCANOSS_SARIF_REPORT_FILE_NAME = "results.sarif"

        // Markdown Jira report file names
        SCANOSS_COPYLEFT_JIRA_REPORT_MD = "scanoss-copyleft-jira_report.md"
        SCANOSS_UNDECLARED_JIRA_REPORT_MD = "scanoss-undeclared-components-jira-report.md"
    }

    stages {
        stage('SCANOSS') {
            agent {
                docker {
                    image params.SCANOSS_CLI_DOCKER_IMAGE
                    reuseNode true
                }
            }
            steps {
               script {
                   // Policies status
                   env.COPYLEFT_POLICY_STATUS = '0'
                   env.UNDECLARED_POLICY_STATUS = '0'

                   def buildNumber = env.BUILD_NUMBER
                   def pipelineName = env.JOB_NAME

                   // Ejecutar Escaneo Principal
                   scan()

                   // Formatos de Salida (SBOMs tradicionales)
                   convertToCycloneDX()
                   convertToSPDX()
                   convertToCSV()
                   
                   // GENERAR FORMATO SARIF PARA LOS GRÁFICOS
                   convertToSARIF()

                   copyleftPolicyCheck()
                   undeclaredComponentsPolicyCheck()
                   
                   echo "[ Copyleft status ]: ${env.COPYLEFT_POLICY_STATUS}"
                   echo "[ Undeclared Components status ]: ${env.UNDECLARED_POLICY_STATUS}"

                    if (params.CREATE_JIRA_ISSUE) {
                        echo "Create Jira Issue: ENABLED"
                        if (env.COPYLEFT_POLICY_STATUS == '1') {
                            createJiraMarkdownCopyleftReport()
                            createJiraTicket("Copyleft licenses found - ${pipelineName}/${buildNumber}", env.SCANOSS_COPYLEFT_JIRA_REPORT_MD)
                        }
                        if (env.UNDECLARED_POLICY_STATUS == '1') {
                            createJiraMarkdownUndeclaredComponentReport()
                            createJiraTicket("Undeclared components found - ${pipelineName}/${buildNumber}", env.SCANOSS_UNDECLARED_JIRA_REPORT_MD)
                        }
                    }

                   // MOSTRAR GRÁFICOS EN JENKINS (Plugin Warnings Next Generation)
                   recordIssues(
                       tools: [sarif(pattern: env.SCANOSS_SARIF_REPORT_FILE_NAME, id: 'scanoss', name: 'SCANOSS Analysis')],
                       qualityGates: [
                           [threshold: 1, type: 'TOTAL', criticality: 'UNSTABLE']
                       ]
                   )

                   if (env.COPYLEFT_POLICY_STATUS == '1' || env.UNDECLARED_POLICY_STATUS == '1') {
                       currentBuild.result = 'UNSTABLE'
                   }
                }
            }
        }
    }
}

def convertToSARIF() {
    script {
        def cmd = [
            'scanoss-py',
            'convert',
            '--input', env.SCANOSS_RESULTS_OUTPUT_FILE_NAME,
            '--format', 'sarif',
            '--output', env.SCANOSS_SARIF_REPORT_FILE_NAME
        ]
        if(params.DEBUG) { cmd << "--debug" }
        sh(script: cmd.join(' '), returnStatus: true)
        uploadArtifact(env.SCANOSS_SARIF_REPORT_FILE_NAME)
    }
}

def createJiraMarkdownUndeclaredComponentReport() {
    script {
        def cmd = [
            'scanoss-py', 'insp', 'undeclared',
            '--input', env.SCANOSS_RESULTS_OUTPUT_FILE_NAME,
            '--output', 'scanoss-undeclared-components-jira.md',
            '--status', 'scanoss-undeclared-status-jira.md',
            '-f', 'jira_md']
        def exitCode = sh(script: cmd.join(' '), returnStatus: true)
        if (exitCode == 0) {
            sh """
                cat scanoss-undeclared-components-jira.md scanoss-undeclared-status-jira.md > "${env.SCANOSS_UNDECLARED_JIRA_REPORT_MD}"
                chmod 644 "${env.SCANOSS_UNDECLARED_JIRA_REPORT_MD}"
            """
        }
    }
}

def createJiraMarkdownCopyleftReport(){
    script {
        def cmd = [
            'scanoss-py', 'insp', 'copyleft',
            '--input', env.SCANOSS_RESULTS_OUTPUT_FILE_NAME,
            '--output', env.SCANOSS_COPYLEFT_JIRA_REPORT_MD,
            '-f', 'jira_md']
        cmd.addAll(buildCopyleftArgs())
        if(params.DEBUG) { cmd << "--debug" }
        sh(script: cmd.join(' '), returnStatus: true)
    }
}

def undeclaredComponentsPolicyCheck() {
    script {
        def cmd = [
            'scanoss-py', 'insp', 'undeclared',
            '--input', env.SCANOSS_RESULTS_OUTPUT_FILE_NAME,
            '--output', 'scanoss-undeclared-components.md',
            '--status', 'scanoss-undeclared-status.md',
            '-f', 'md']
        if(params.DEBUG) { cmd << "--debug" }
        def exitCode = sh(script: cmd.join(' '), returnStatus: true)
        if (exitCode == 1) {
            echo "No Undeclared components were found"
        } else {
            echo "Undeclared Components were found"
            env.UNDECLARED_POLICY_STATUS = '1'
            sh """
                cat scanoss-undeclared-components.md > "${env.SCANOSS_UNDECLARED_REPORT_MD}"
                cat scanoss-undeclared-status.md >> "${env.SCANOSS_UNDECLARED_REPORT_MD}"
                chmod 644 "${env.SCANOSS_UNDECLARED_REPORT_MD}"
            """
            uploadArtifact(env.SCANOSS_UNDECLARED_REPORT_MD)
        }
    }
}

def copyleftPolicyCheck() {
    script {
        def cmd = [
            'scanoss-py', 'insp', 'copyleft',
            '--input', env.SCANOSS_RESULTS_OUTPUT_FILE_NAME,
            '--output', env.SCANOSS_COPYLEFT_REPORT_MD,
            '-f', 'md']
        cmd.addAll(buildCopyleftArgs())
        if(params.DEBUG) { cmd << "--debug" }
        def exitCode = sh(script: cmd.join(' '), returnStatus: true)
        if (exitCode == 1) {
            echo "No copyleft licenses were found"
        } else {
            echo "Copyleft Licenses were found"
            env.COPYLEFT_POLICY_STATUS = '1'
            uploadArtifact(env.SCANOSS_COPYLEFT_REPORT_MD)
        }
    }
}

def scan() {
    withCredentials([string(credentialsId: params.SCANOSS_API_TOKEN_ID, variable: 'SCANOSS_API_TOKEN')]) {
        script {
            def cmd = []
            cmd << "scanoss-py scan"
            cmd << validateScanPath(params.SCAN_PATH)
            cmd << "--apiurl ${params.SCANOSS_API_URL}"
            if(env.SCANOSS_API_TOKEN) { cmd << "--key ${SCANOSS_API_TOKEN}" }
            if(params.SKIP_SNIPPET) { cmd << "-S" }
            if(params.SCANOSS_SETTINGS) {
               cmd << "--settings ${params.SETTINGS_FILE_PATH}"
            } else {
               cmd << "-stf"
            }
            if(params.DEPENDENCY_ENABLED) { cmd << buildDependencyScopeArgs() }
            cmd << "--output ${env.SCANOSS_RESULTS_OUTPUT_FILE_NAME}"
            if(params.DEBUG) { cmd << "--debug" }
            
            sh(script: cmd.join(' '), returnStatus: true)
            uploadArtifact(env.SCANOSS_RESULTS_OUTPUT_FILE_NAME)
        }
    }
}

def convertToCycloneDX() {
    script {
        def cmd = ['scanoss-py', 'convert', '--input', env.SCANOSS_RESULTS_OUTPUT_FILE_NAME, '--format', 'cyclonedx', '--output', env.SCANOSS_CYCLONEDX_REPORT_FILE_NAME]
        if(params.DEBUG) { cmd << "--debug" }
        sh(script: cmd.join(' '), returnStatus: true)
        uploadArtifact(env.SCANOSS_CYCLONEDX_REPORT_FILE_NAME)
    }
}

def convertToSPDX() {
    script {
        def cmd = ['scanoss-py', 'convert', '--input', env.SCANOSS_RESULTS_OUTPUT_FILE_NAME, '--format', 'spdxlite', '--output', env.SCANOSS_SPDX_REPORT_FILE_NAME]
        if(params.DEBUG) { cmd << "--debug" }
        sh(script: cmd.join(' '), returnStatus: true)
        uploadArtifact(env.SCANOSS_SPDX_REPORT_FILE_NAME)
    }
}

def convertToCSV() {
    script {
        def cmd = ['scanoss-py', 'convert', '--input', env.SCANOSS_RESULTS_OUTPUT_FILE_NAME, '--format', 'csv', '--output', env.SCANOSS_CSV_REPORT_FILE_NAME]
        if(params.DEBUG) { cmd << "--debug" }
        sh(script: cmd.join(' '), returnStatus: true)
        uploadArtifact(env.SCANOSS_CSV_REPORT_FILE_NAME)
    }
}

def uploadArtifact(artifactPath) {
    archiveArtifacts artifacts: artifactPath, allowEmptyArchive: true
}

def List<String> buildDependencyScopeArgs() {
    if (params.DEPENDENCY_SCOPE_EXCLUDE != '') { return ['--dep-scope-exc', params.DEPENDENCY_SCOPE_EXCLUDE] }
    if (params.DEPENDENCY_SCOPE_INCLUDE != '') { return ['--dep-scope-inc', params.DEPENDENCY_SCOPE_INCLUDE] }
    if (params.DEPENDENCY_SCOPE == 'prod') { return ['--dep-scope', 'prod'] }
    if (params.DEPENDENCY_SCOPE == 'dev') { return ['--dep-scope', 'dev'] }
    return []
}

def List<String> buildCopyleftArgs() {
    if (params.LICENSES_COPYLEFT_EXPLICIT != '') { return ['--explicit', params.LICENSES_COPYLEFT_EXPLICIT] }
    if (params.LICENSES_COPYLEFT_INCLUDE != '') { return ['--include', params.LICENSES_COPYLEFT_INCLUDE] }
    if (params.LICENSES_COPYLEFT_EXCLUDE != '') { return ['--exclude', params.LICENSES_COPYLEFT_EXCLUDE] }
    return []
}

def validateScanPath(String scanPath) {
    if (!scanPath || scanPath.trim() == '') return '.'
    def normalized = scanPath.replace('\\', '/')
    if (normalized.startsWith('/') || normalized ==~ /^[a-zA-Z]:.*/ || normalized.contains('..')) return '.'
    return normalized.startsWith('./') ? normalized.substring(2) : normalized
}

def createJiraTicket(String title, String filePath) {
    // Se mantiene la función por integridad estructural pero no se ejecutará si CREATE_JIRA_ISSUE es false
    echo "Simulating Jira Ticket creation for: ${title}"
}