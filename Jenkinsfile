pipeline {
    agent any // El nodo principal debe tener Docker instalado

    parameters {
        string(name: 'SCANOSS_API_TOKEN_ID', defaultValue:"scanoss-token", description: 'The reference ID for the SCANOSS API TOKEN credential')
        string(name: 'SCANOSS_CLI_DOCKER_IMAGE', defaultValue:"ghcr.io/scanoss/scanoss-py-jenkins:v1.46.0", description: 'SCANOSS CLI Docker Image')
        string(name: 'SCANOSS_API_URL', defaultValue:"https://api.osskb.org/scan/direct", description: 'SCANOSS API URL')
        string(name: 'SCAN_PATH', defaultValue: '.', description: 'Path to scan')
        booleanParam(name: 'SKIP_SNIPPET', defaultValue: false, description: 'Skip snippets')
        booleanParam(name: 'SCANOSS_SETTINGS', defaultValue: true, description: 'Use settings file')
        string(name: 'SETTINGS_FILE_PATH', defaultValue: 'scanoss.json', description: 'Settings file path')
        booleanParam(name: 'DEPENDENCY_ENABLED', defaultValue: false, description: 'Scan dependencies')
        string(name: 'DEPENDENCY_SCOPE', defaultValue: '', description: 'prod|dev')
        string(name: 'DEPENDENCY_SCOPE_INCLUDE', defaultValue: '', description: 'Include scopes')
        string(name: 'DEPENDENCY_SCOPE_EXCLUDE', defaultValue: '', description: 'Exclude scopes')
        string(name: 'LICENSES_COPYLEFT_INCLUDE', defaultValue: '', description: 'Include licenses')
        string(name: 'LICENSES_COPYLEFT_EXCLUDE', defaultValue: '', description: 'Exclude licenses')
        string(name: 'LICENSES_COPYLEFT_EXPLICIT', defaultValue: '', description: 'Explicit licenses')
        string(name: 'JIRA_CREDENTIALS', defaultValue:"jira-credentials" , description: 'Jira credentials')
        string(name: 'JIRA_URL', defaultValue:"" , description: 'Jira URL')
        string(name: 'JIRA_PROJECT_KEY', defaultValue:"" , description: 'Jira Project Key')
        booleanParam(name: 'CREATE_JIRA_ISSUE', defaultValue: false, description: 'Enable Jira reporting')
        booleanParam(name: 'ABORT_ON_POLICY_FAILURE', defaultValue: false, description: 'Abort on failure')
        booleanParam(name: 'DEBUG', defaultValue: false , description: 'Enable debugging')
    }

    environment {
        SCANOSS_COPYLEFT_REPORT_MD = "scanoss-copyleft-report.md"
        SCANOSS_UNDECLARED_REPORT_MD = "scanoss-undeclared-report.md"
        SCANOSS_RESULTS_OUTPUT_FILE_NAME = "results.json"
        SCANOSS_CYCLONEDX_REPORT_FILE_NAME = "scanoss-cyclonedx.json"
        SCANOSS_SPDX_REPORT_FILE_NAME = "scanoss-spdx.json"
        SCANOSS_CSV_REPORT_FILE_NAME = "scanoss-sbom.csv"
        SCANOSS_SARIF_REPORT_FILE_NAME = "results.sarif"
        SCANOSS_COPYLEFT_JIRA_REPORT_MD = "scanoss-copyleft-jira_report.md"
        SCANOSS_UNDECLARED_JIRA_REPORT_MD = "scanoss-undeclared-components-jira-report.md"
    }

    stages {
        stage('SCANOSS Analysis') {
            agent {
                docker {
                    image params.SCANOSS_CLI_DOCKER_IMAGE
                    // El flag -u 0:0 asegura que no haya líos de permisos de escritura en el workspace
                    args '-u 0:0 --platform linux/arm64' 
                    reuseNode true
                }
            }
            steps {
               script {
                   env.COPYLEFT_POLICY_STATUS = '0'
                   env.UNDECLARED_POLICY_STATUS = '0'

                   // 1. Escaneo
                   scan()

                   // 2. Conversiones
                   convertToCycloneDX()
                   convertToSPDX()
                   convertToCSV()
                   convertToSARIF()

                   // 3. Checks de Políticas
                   copyleftPolicyCheck()
                   undeclaredComponentsPolicyCheck()
                   
                   // 4. Reporte Visual en Jenkins
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

// --- Funciones de soporte (igual que las anteriores) ---

def convertToSARIF() {
    script {
        sh "scanoss-py convert --input ${env.SCANOSS_RESULTS_OUTPUT_FILE_NAME} --format sarif --output ${env.SCANOSS_SARIF_REPORT_FILE_NAME}"
        uploadArtifact(env.SCANOSS_SARIF_REPORT_FILE_NAME)
    }
}

def undeclaredComponentsPolicyCheck() {
    script {
        def exitCode = sh(script: "scanoss-py insp undeclared --input ${env.SCANOSS_RESULTS_OUTPUT_FILE_NAME} --output scanoss-undeclared-components.md --status scanoss-undeclared-status.md -f md", returnStatus: true)
        if (exitCode != 1) {
            env.UNDECLARED_POLICY_STATUS = '1'
            sh "cat scanoss-undeclared-components.md scanoss-undeclared-status.md > ${env.SCANOSS_UNDECLARED_REPORT_MD}"
            uploadArtifact(env.SCANOSS_UNDECLARED_REPORT_MD)
        }
    }
}

def copyleftPolicyCheck() {
    script {
        def cmd = "scanoss-py insp copyleft --input ${env.SCANOSS_RESULTS_OUTPUT_FILE_NAME} --output ${env.SCANOSS_COPYLEFT_REPORT_MD} -f md"
        def exitCode = sh(script: cmd, returnStatus: true)
        if (exitCode != 1) {
            env.COPYLEFT_POLICY_STATUS = '1'
            uploadArtifact(env.SCANOSS_COPYLEFT_REPORT_MD)
        }
    }
}

def scan() {
    withCredentials([string(credentialsId: params.SCANOSS_API_TOKEN_ID, variable: 'SCANOSS_API_TOKEN')]) {
        script {
            def cmd = "scanoss-py scan ${validateScanPath(params.SCAN_PATH)} --apiurl ${params.SCANOSS_API_URL} --output ${env.SCANOSS_RESULTS_OUTPUT_FILE_NAME}"
            if(env.SCANOSS_API_TOKEN) { cmd += " --key ${SCANOSS_API_TOKEN}" }
            if(params.SKIP_SNIPPET) { cmd += " -S" }
            sh script: cmd, returnStatus: true
            uploadArtifact(env.SCANOSS_RESULTS_OUTPUT_FILE_NAME)
        }
    }
}

def convertToCycloneDX() { script { sh "scanoss-py convert --input ${env.SCANOSS_RESULTS_OUTPUT_FILE_NAME} --format cyclonedx --output ${env.SCANOSS_CYCLONEDX_REPORT_FILE_NAME}"; uploadArtifact(env.SCANOSS_CYCLONEDX_REPORT_FILE_NAME) } }
def convertToSPDX() { script { sh "scanoss-py convert --input ${env.SCANOSS_RESULTS_OUTPUT_FILE_NAME} --format spdxlite --output ${env.SCANOSS_SPDX_REPORT_FILE_NAME}"; uploadArtifact(env.SCANOSS_SPDX_REPORT_FILE_NAME) } }
def convertToCSV() { script { sh "scanoss-py convert --input ${env.SCANOSS_RESULTS_OUTPUT_FILE_NAME} --format csv --output ${env.SCANOSS_CSV_REPORT_FILE_NAME}"; uploadArtifact(env.SCANOSS_CSV_REPORT_FILE_NAME) } }

def uploadArtifact(artifactPath) {
    archiveArtifacts artifacts: artifactPath, allowEmptyArchive: true
}

def validateScanPath(String scanPath) {
    if (!scanPath || scanPath.trim() == '' || scanPath.contains('..')) return '.'
    return scanPath
}

// Jira dummy (Para no romper el script)
def createJiraMarkdownUndeclaredComponentReport() {}
def createJiraMarkdownCopyleftReport() {}
def createJiraTicket(t, f) {}