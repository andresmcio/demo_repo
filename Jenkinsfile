pipeline {
    agent any // El nodo debe tener Docker disponible en el host

    parameters {
        string(name: 'SCANOSS_API_TOKEN_ID', defaultValue:"scanoss-token", description: 'scanoss-token')
        string(name: 'SCANOSS_CLI_DOCKER_IMAGE', defaultValue:"ghcr.io/scanoss/scanoss-py-jenkins:v1.46.0", description: 'Docker Image')
        string(name: 'SCANOSS_API_URL', defaultValue:"https://api.osskb.org/scan/direct", description: 'https://api.scanoss.com')
        string(name: 'SCAN_PATH', defaultValue: '.', description: 'Path to scan')
        booleanParam(name: 'DEBUG', defaultValue: false , description: 'Enable debugging')
    }

    environment {
        SCANOSS_RESULTS_JSON = "results.json"
        SCANOSS_SARIF = "results.sarif"
    }

    stages {
        stage('SCANOSS Execution') {
            agent {
                docker {
                    image params.SCANOSS_CLI_DOCKER_IMAGE
                    // Forzamos arquitectura ARM64 para tu M1 Pro y usuario root para evitar líos de permisos
                    args '-u 0:0 --platform linux/arm64'
                    reuseNode true
                }
            }
            steps {
               script {
                   echo "--- Iniciando Escaneo SCANOSS ---"
                   
                   // Ejecución del escaneo
                   scan()

                   // Generación de SARIF para los gráficos
                   sh "scanoss-py convert --input ${env.SCANOSS_RESULTS_JSON} --format sarif --output ${env.SCANOSS_SARIF}"
                   
                   // Guardar artefactos
                   archiveArtifacts artifacts: "${env.SCANOSS_RESULTS_JSON}, ${env.SCANOSS_SARIF}", allowEmptyArchive: true

                   // Renderizar los gráficos de Warnings Next Gen
                   recordIssues(
                       tools: [sarif(pattern: env.SCANOSS_SARIF, id: 'scanoss', name: 'SCANOSS SBOM Analysis')],
                       qualityGates: [[threshold: 1, type: 'TOTAL', criticality: 'UNSTABLE']]
                   )
                }
            }
        }
    }
}

def scan() {
    withCredentials([string(credentialsId: params.SCANOSS_API_TOKEN_ID, variable: 'SCANOSS_API_TOKEN')]) {
        script {
            def cmd = "scanoss-py scan . --apiurl ${params.SCANOSS_API_URL} --output ${env.SCANOSS_RESULTS_JSON}"
            if(env.SCANOSS_API_TOKEN) { cmd += " --key ${SCANOSS_API_TOKEN}" }
            if(params.DEBUG) { cmd += " --debug" }
            
            // returnStatus: true evita que el pipeline muera si el escaneo detecta algo (queremos llegar a los gráficos)
            sh script: cmd, returnStatus: true
        }
    }
}